#pragma once
#include <stdint.h>
#include "font.h"


#define RGB565(r,g,b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

#define DISPLAY_WIDTH  480
#define DISPLAY_HEIGHT 320 

#define NDSU_GREEN  RGB565(0,102,51)
#define NDSU_YELLOW RGB565(255,204,0)

/**
 * @brief Initializes the display subsystem.
 *
 * Performs all required setup for the display hardware and prepares the
 * graphics system for use.
 * This function must be called before using any other display functions.
 */
void display_init();

/**
 * @brief Sets the display background color.
 *
 * Fills the entire display with the specified color value.
 * This operation typically clears any previously drawn graphics.
 *
 * @param color The 16-bit color value used to fill the background.
 *
 */
void display_background_color(uint16_t color);

/**
 * @brief Draws a filled rectangular box on the display.
 *
 * The box is drawn starting at the specified top-left coordinate
 * and extends to the specified width and height.
 *
 * @param x X coordinate of the top-left corner of the box in pixels.
 * @param y Y coordinate of the top-left corner of the box in pixels.
 * @param w Width of the box in pixels.
 * @param h Height of the box in pixels.
 * @param color 16-bit color used to fill the box
 */
void display_draw_box(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/**
 * @brief Draws a text string on the display.
 *
 * Renders the provided null-terminated string at the specified
 * screen position using the selected font size and color.
 *
 * @param text Pointer to the null-terminated string to display.
 * @param x X coordinate where the text begins.
 * @param y Y coordinate of the text position baseline
 * @param color 16-bit color used to render the text.
 * @param size Font size used for rendering the text (9,12,18,24pt).
 */
void display_draw_text(const char* text, uint16_t x, uint16_t y, uint16_t color, FontSize size);

/**
 * @brief Calculates the rendered size of a text string.
 *
 * Determines the width and height that the provided string would occupy
 * when rendered with the specified font size.
 *
 * @param text Pointer to the null-terminated string to measure.
 * @param size Font size used to calculate the dimensions.
 * @param w Pointer to a variable that will receive the calculated width in pixels.
 * @param h Pointer to a variable that will receive the calculated height in pixels.
 * @param lower Pointer to a variable that will receive the descender height in pixels
 *              (the portion of characters that extend below the baseline, such as 'g' or 'y').
 */
void display_get_text_size(char* text, FontSize size, uint16_t* w, uint8_t* h, uint8_t* lower);
