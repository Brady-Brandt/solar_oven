#pragma once
#include <stdint.h>


typedef uint8_t RGB666[3];

#define RGB_TO_RGB666(red,green,blue) {(red * 63/ 255) << 2, (green * 63/ 255) << 2, (blue * 63/ 255) << 2} 

#define DISPLAY_WIDTH  480
#define DISPLAY_HEIGHT 320 


void display_init();


void display_background_color(uint8_t r, uint8_t g, uint8_t b);

