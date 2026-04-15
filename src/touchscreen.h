#pragma once
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initializes the touchscreen hardware and prepares it for use. 
 */
void touchscreen_init();

/**
 * @brief Retrieves the current touch coordinates from the touchscreen.
 *
 * Attempts to read a touch point from the touchscreen and stores the
 * corresponding X and Y coordinates in the provided pointers.
 *
 * @param[out] x Pointer to a variable where the X coordinate will be stored.
 * @param[out] y Pointer to a variable where the Y coordinate will be stored.
 *
 * @return true if a valid touch point was detected and coordinates were written.
 * @return false if no touch is currently detected or the read failed. 
 */
bool touchscreen_get_point(uint16_t* x, uint16_t* y);
