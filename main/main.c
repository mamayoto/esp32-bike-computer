#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include "rom/ets_sys.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "u8g2.h"
#include "u8g2_esp32_hal.h"
#include "DHT22.h"

#define ESP_INTR_FLAG_DEFAULT 0

#define INT_BUTTON 0
#define INT_LED
#define PIN_DHT22 33
#define PIN_CLK 18
#define PIN_MOSI 23
#define PIN_RESET 4
#define PIN_DC 25
#define PIN_CS 26


static xQueueHandle temp_queue = NULL;
static xQueueHandle hum_queue = NULL;
char screen_select = 0;
TaskHandle_t ISR = NULL;


void vDHT22(void *pvParameter) {
	setDHTgpio(PIN_DHT22);
	while(1) {
		int ret = readDHT();
		errorHandler(ret);

		float temp = getTemperature();
		float hum = getHumidity();

		if(!xQueueSend(temp_queue, &temp, portMAX_DELAY)) {
			ESP_LOGE("vDHT22", "failed to send temp to queue within %dms", portMAX_DELAY);
		}

		if(!xQueueSend(hum_queue, &hum, portMAX_DELAY)) {
			ESP_LOGE("vDHT22", "failed to send humidity to queue within %dms", portMAX_DELAY);
		}

		vTaskDelay(2000 / portTICK_RATE_MS);
	}
}


void vU8G2(void *pvParameter) {
	// init esp32 hardware abstraction layer for u8g2
	u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
	u8g2_esp32_hal.clk   = PIN_CLK;
	u8g2_esp32_hal.mosi  = PIN_MOSI;
	u8g2_esp32_hal.cs    = PIN_CS;
	u8g2_esp32_hal.dc    = PIN_DC;
	u8g2_esp32_hal.reset = PIN_RESET;
	u8g2_esp32_hal_init(u8g2_esp32_hal);

	u8g2_t u8g2;
	u8g2_Setup_sh1106_128x64_noname_f(
			&u8g2,
			U8G2_R2,
			u8g2_esp32_spi_byte_cb,
			u8g2_esp32_gpio_and_delay_cb);
	u8g2_InitDisplay(&u8g2);
	u8g2_SetPowerSave(&u8g2, 0);

	float temp;
	float hum;
	char temp_str[10];
	char hum_str[10];

	while(1) {
		if(!xQueueReceive(temp_queue, &temp, portMAX_DELAY)) {
			ESP_LOGE("vU8G2", "failed to receive temperature data from queue within %dms", portMAX_DELAY);
		}

		if(!xQueueReceive(hum_queue, &hum, portMAX_DELAY)) {
			ESP_LOGE("vU8G2", "failed to receive humidity data from queue within %dms", portMAX_DELAY);
		}

		snprintf(temp_str, sizeof(temp_str), "%.1f°C", temp);
		snprintf(hum_str, sizeof(hum_str), "%.1f%%", hum);


		u8g2_ClearBuffer(&u8g2);
		u8g2_SetFont(&u8g2, u8g2_font_9x18B_tf);
		u8g2_SetFontPosTop(&u8g2);

		switch (screen_select) {
			case 0:
				u8g2_DrawUTF8(&u8g2, 0, 0, temp_str);
				u8g2_DrawUTF8(&u8g2, 0, 20, hum_str);
				u8g2_DrawRBox(&u8g2, 0, 52, 32, 15, 5);
				u8g2_DrawRFrame(&u8g2, 31, 52, 33, 15, 5);
				u8g2_DrawRFrame(&u8g2, 63, 52, 33, 15, 5);
				u8g2_DrawRFrame(&u8g2, 95, 52, 33, 15, 5);
				break;

			case 1:
				u8g2_DrawUTF8(&u8g2, 0, 0, temp_str);
				u8g2_DrawUTF8(&u8g2, 0, 20, hum_str);
				u8g2_DrawRFrame(&u8g2, 0, 52, 32, 15, 5);
				u8g2_DrawRBox(&u8g2, 31, 52, 33, 15, 5);
				u8g2_DrawRFrame(&u8g2, 63, 52, 33, 15, 5);
				u8g2_DrawRFrame(&u8g2, 95, 52, 33, 15, 5);
				break;

			case 2:
				u8g2_DrawUTF8(&u8g2, 0, 0, temp_str);
				u8g2_DrawUTF8(&u8g2, 0, 20, hum_str);
				u8g2_DrawRFrame(&u8g2, 0, 52, 32, 15, 5);
				u8g2_DrawRFrame(&u8g2, 31, 52, 33, 15, 5);
				u8g2_DrawRBox(&u8g2, 63, 52, 33, 15, 5);
				u8g2_DrawRFrame(&u8g2, 95, 52, 33, 15, 5);
				break;

			case 3:
				u8g2_DrawUTF8(&u8g2, 0, 0, temp_str);
				u8g2_DrawUTF8(&u8g2, 0, 20, hum_str);
				u8g2_DrawRFrame(&u8g2, 0, 52, 32, 15, 5);
				u8g2_DrawRFrame(&u8g2, 31, 52, 33, 15, 5);
				u8g2_DrawRFrame(&u8g2, 63, 52, 33, 15, 5);
				u8g2_DrawRBox(&u8g2, 95, 52, 33, 15, 5);
				break;
		}

		u8g2_SendBuffer(&u8g2);
		vTaskDelay(500 / portTICK_RATE_MS);
	}
}


void vButton(void *pvParameter) {
	while(1){
		vTaskSuspend(NULL);
		ESP_LOGI("vButton", "button pressed");

		if (screen_select < 3) {
			screen_select++;
		}
		else {
			screen_select = 0;
		}
	}
}

void IRAM_ATTR button_isr_handler(void* arg) {
	xTaskResumeFromISR(ISR);
}


void app_main() {
	gpio_pad_select_gpio(INT_BUTTON);
	gpio_set_direction(INT_BUTTON, GPIO_MODE_INPUT);
	gpio_set_intr_type(INT_BUTTON, GPIO_INTR_NEGEDGE);
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	gpio_isr_handler_add(INT_BUTTON, button_isr_handler, NULL);


	temp_queue = xQueueCreate(1, sizeof(float));
	hum_queue = xQueueCreate(1, sizeof(float));

	vTaskDelay(1000 / portTICK_RATE_MS );
	xTaskCreate(&vDHT22, "vDHT22", 2048, NULL, 5, NULL);
	xTaskCreate(&vU8G2, "vU8G2", 4096, NULL, 5, NULL);
	xTaskCreate(vButton, "vButton", 4096, NULL , 10, &ISR);
}
