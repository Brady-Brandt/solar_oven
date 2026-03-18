#pragma once
#include "font.h"
#include <stdint.h>


#define NO_BACKGROUND_COLOR 0xFFFF
#define TASKBAR_Y 310

extern uint8_t TEMP_L1;
extern uint8_t TEMP_L2;
extern uint8_t TEMP_L3;


typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint8_t h;
    uint8_t lower_bound; //min height of text, used for clearing the label
    uint16_t fg;
    uint16_t bg;
    FontSize font;
    char* txt;
} Label;


typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
    uint16_t color;
} Screen;

/**
 * @brief Clears the UI display using the specified background color.
 *
 * This function resets the entire screen and fills it with the given
 * background color. It is typically called before drawing a new UI
 * screen to ensure that previous content is removed.
 *
 * @param bg The background color used to clear the screen (16-bit color value).
 */
void ui_clear(uint16_t bg);

/**
 * @brief Draws the temperature screen with initial sensor values.
 *
 * This function renders the temperature display layout and populates it
 * with the provided sensor readings. It is intended to be called when
 * the temperature UI screen is first displayed.
 *
 * @param s1 Initial temperature value for sensor 1.
 * @param s2 Initial temperature value for sensor 2.
 * @param s3 Initial temperature value for sensor 3.
 */
void ui_draw_temperature_screen(uint16_t s1, uint16_t s2, uint16_t s3);

/**
 * @brief Updates the displayed temperature for a specific label.
 *
 * This function updates the temperature value shown on the screen for
 * a given sensor label without redrawing the entire screen.
 *
 * @param lbl Identifier of the temperature label to update
 * @param temp New temperature value to display.
 */
void ui_update_temperature(uint8_t lbl, uint16_t temp);

/**
 * @brief Updates the time 
 *
 * This function updates the time in the right left corner using the rtc
 * that was synced via an ntp server
 *
 */
void ui_display_time();

/**
 * @brief Displays the wifi status
 *
 * This function gets and displays the wifi status in the lower right corner
 *
*/
void ui_display_wifi_status();
