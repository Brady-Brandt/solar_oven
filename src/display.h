#pragma once
#include <stdint.h>

typedef uint8_t RGB666[3];

#define RGB_TO_RGB666(red,green,blue) {(red * 63/ 255) << 2, (green * 63/ 255) << 2, (blue * 63/ 255) << 2}

#define DISPLAY_WIDTH  480
#define DISPLAY_HEIGHT 320 

#define NDSU_GREEN  0,102,51
#define NDSU_YELLOW 255,204,0

void display_init();

void display_background_color(uint8_t r, uint8_t g, uint8_t b);

void display_draw_box(uint16_t x, uint16_t y,uint16_t w, uint16_t h, uint8_t* color);

void display_draw_text(const char* text, uint16_t x, uint16_t y);
