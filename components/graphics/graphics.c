#include "graphics.h"

void draw_page1(u8g2_t *u8g2, const char *temp, const char *hum) {
	u8g2_DrawUTF8(u8g2, 0, 1, temp);
	u8g2_DrawUTF8(u8g2, 0, 20, hum);
	u8g2_DrawRBox(u8g2, 0, 52, 32, 15, 5);
	u8g2_DrawRFrame(u8g2, 31, 52, 33, 15, 5);
	u8g2_DrawRFrame(u8g2, 63, 52, 33, 15, 5);
	u8g2_DrawRFrame(u8g2, 95, 52, 33, 15, 5);
}

void draw_page2(u8g2_t *u8g2) {
	u8g2_DrawUTF8(u8g2, 0, 20, "page 2");
	u8g2_DrawRFrame(u8g2, 0, 52, 32, 15, 5);
	u8g2_DrawRBox(u8g2, 31, 52, 33, 15, 5);
	u8g2_DrawRFrame(u8g2, 63, 52, 33, 15, 5);
	u8g2_DrawRFrame(u8g2, 95, 52, 33, 15, 5);
}

void draw_page3(u8g2_t *u8g2) {
	u8g2_DrawUTF8(u8g2, 0, 20, "page 3");
	u8g2_DrawRFrame(u8g2, 0, 52, 32, 15, 5);
	u8g2_DrawRFrame(u8g2, 31, 52, 33, 15, 5);
	u8g2_DrawRBox(u8g2, 63, 52, 33, 15, 5);
	u8g2_DrawRFrame(u8g2, 95, 52, 33, 15, 5);
}

void draw_page4(u8g2_t *u8g2) {
	u8g2_DrawUTF8(u8g2, 0, 20, "page 4");
	u8g2_DrawRFrame(u8g2, 0, 52, 32, 15, 5);
	u8g2_DrawRFrame(u8g2, 31, 52, 33, 15, 5);
	u8g2_DrawRFrame(u8g2, 63, 52, 33, 15, 5);
	u8g2_DrawRBox(u8g2, 95, 52, 33, 15, 5);
}
