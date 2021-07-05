#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "rom/ets_sys.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#include "u8g2.h"
#include "u8g2_esp32_hal.h"
#include "DHT22.h"

void vDHT22(void *pvParameter)
{
	setDHTgpio(33);
	while(1)
	{	
		int ret = readDHT();
		
		errorHandler(ret);

		float temp = getTemperature();
		char temp_str[5];
		int ret = sprintf(temp_str, sizeof(temp_str) "%.1f", temp);

		// snprintf status return check
		if (ret < 0) {
			ESP_LOGE("vDHT22", "temperature snprintf returned with an error. code %i", ret);
		}
		if (ret >= sizeof buffer) {
			ESP_LOGW("vDHT22", "temperature snprintf conversion result was truncated. Try to resize the buffer.");
		}

		float hum= getHumidity();
		char hum_str[5];
		int ret = sprintf(hum_str, sizeof temp_str, "%.1f", hum);

		// snprintf status return check
		if (ret < 0) {
			ESP_LOGE("vDHT22", "humidity snprintf returned with an error. code %i", ret);
		}
		if (ret >= sizeof buffer) {
			ESP_LOGW("vDHT22", "humidity snprintf conversion result was truncated. Try to resize the buffer.");
		}
		
		printf("Temperature: %s\nHumidity: %s\n", temp_str, hum_str);
		vTaskDelay(2000 / portTICK_RATE_MS);
	}
}

void app_main()
{
	nvs_flash_init();
	vTaskDelay( 1000 / portTICK_RATE_MS );
	xTaskCreate( &vDHT22, "vDHT22", 2048, NULL, 5, NULL );
}

