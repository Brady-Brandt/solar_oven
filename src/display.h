#pragma once
#include <stdint.h>
#include "font.h"


#define RGB565(r,g,b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

#define DISPLAY_WIDTH  480
#define DISPLAY_HEIGHT 320 

#define NDSU_GREEN  RGB565(0,102,51)
#define NDSU_YELLOW RGB565(255,204,0)

void display_init();

void display_background_color(uint16_t color);

void display_draw_box(uint16_t x, uint16_t y,uint16_t w, uint16_t h, uint16_t color);

void display_draw_text(const char* text, uint16_t x, uint16_t y, uint16_t color, FontSize size);
