#include "debug.h"


#ifdef DEBUG 
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include <stdarg.h>

//may use this pin for visual debugging
#define LED_PIN  16

void debug_init(){
    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
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
    gpio_put(LED_PIN, 1);
}

void debug_pin_off(){
    gpio_put(LED_PIN, 0);
}

void debug_pin_put(unsigned int value){
    gpio_put(LED_PIN, value);
}

#endif
