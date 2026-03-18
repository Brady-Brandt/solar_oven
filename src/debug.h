#pragma once

#ifdef DEBUG
/**
 * @brief Initializes the debug subsystem.
 */
void debug_init();

/**
 * @brief Prints a formatted debug message.
 *
 * Outputs a formatted string using a printf-style interface.
 * Intended for general debugging messages.
 *
 * @param fmt Format string (printf-style).
 * @param ... Additional arguments matching the format specifiers.
 */
void debug_print(const char* fmt, ...);

/**
 * @brief Prints an informational debug message.
 *
 * Similar to debug_print, but semantically used for informational
 * messages (e.g., system state, progress updates).
 *
 * @param fmt Format string (printf-style).
 * @param ... Additional arguments matching the format specifiers.
 */
void debug_info(const char* fmt, ...);

/**
 * @brief Prints an error debug message.
 *
 * Similar to debug_print, but intended for reporting errors
 * or abnormal conditions. 
 *
 * @param fmt Format string (printf-style).
 * @param ... Additional arguments matching the format specifiers.
 */
void debug_err(const char* fmt, ...);

/**
 * @brief Sets the debug pin to a high/on state.
 *
 */
void debug_pin_on();

/**
 * @brief Sets the debug pin to a low/off state.
 *
 */
void debug_pin_off();

/**
 * @brief Writes a value to the debug pin.
 *
 * Sets the debug pin output to the specified value.
 * The interpretation of the value depends on the hardware
 * (commonly 0 = low, non-zero = high).
 *
 * @param value Value to write to the debug pin.
 */
void debug_pin_put(unsigned int value);

#else

#define debug_init()               do { } while(0)

#define debug_print(...)           do { } while(0)
#define debug_info(...)            do { } while(0)
#define debug_err(...)             do { } while(0)

#define debug_pin_on()             do { } while(0)
#define debug_pin_off()            do { } while(0)
#define debug_pin_put(value)       do { } while(0)

#endif //DEBUG
