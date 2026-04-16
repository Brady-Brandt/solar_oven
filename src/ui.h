#pragma once
#include <stdint.h>

#define TASKBAR_Y 310

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint8_t h;
    uint8_t lower_bound;
} BoundingBox;


typedef enum {
    SHORT_PRESS,
    MEDIUM_PRESS,
    LONG_PRESS,
} ButtonPress;

typedef struct{
    uint8_t bnd_idx;
    uint8_t is_pressed;
    uint32_t press_start;
    uint32_t last_clicked;
    uint32_t last_seen;
    void (*callback)(ButtonPress press);
} Button;


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
 * @brief Draws the all 3 temperature values
 *
 * This function displays all 3 temperature readings to the screen
 *
 */
void ui_draw_temperature_full();

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

/**
 * @brief Draws the timer and temperature
 *
 * This function displays the timer value and the current
 * temperature in a large font in the middle of teh screen
 *
 */
void ui_draw_timer_and_temp();


/**
 * @brief Displays all UI buttons on the screen.
 *
 * Responsible for rendering the current set of UI buttons
 * based on the active UI state.
 */
void ui_display_btns();

/**
 * @brief Checks UI button states and handles interactions.
 *
 * Processes input events and determines if any UI buttons
 * have been activated, then triggers their associated actions.
 */
void ui_check_btns();

/**
 * @brief Draws the settings screen
 */
void ui_draw_settings_screen();
