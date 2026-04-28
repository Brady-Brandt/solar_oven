#include "debug.h"


#ifdef DEBUG 
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pins.h"
#include <stdarg.h>

void debug_init(){
    stdio_init_all();
    gpio_init(PIN_DEBUG_LED);
    gpio_set_dir(PIN_DEBUG_LED, GPIO_OUT);
}


void debug_print(const char* fmt, ...){
    va_list list;
    va_start(list, fmt);
    stdio_vprintf(fmt, list);
    va_end(list);
}

void debug_info(const char* fmt, ...){
    stdio_printf("INFO: ");
    va_list list;
    va_start(list, fmt);
    stdio_vprintf(fmt, list);
    va_end(list);
}

void debug_err(const char* fmt, ...){
    stdio_printf("ERROR: ");
    va_list list;
    va_start(list, fmt);
    stdio_vprintf(fmt, list);
    va_end(list);
}

void debug_pin_on(){
    gpio_put(PIN_DEBUG_LED, 1);
}

void debug_pin_off(){
    gpio_put(PIN_DEBUG_LED, 0);
}

void debug_pin_put(unsigned int value){
    gpio_put(PIN_DEBUG_LED, value);
}

#endif
