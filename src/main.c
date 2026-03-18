#include <pico/time.h>
#include <pico/types.h>
#include <stdbool.h>
#include <stdint.h>
#include "display.h"
#include "font.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/rtc.h"
#include "debug.h"
#include "wifi.h"
#include "ui.h"

int main() {
    rtc_init();
    debug_init();
    display_init();

    display_background_color(NDSU_GREEN);
    display_draw_text("Solar Oven", 100, 50, NDSU_YELLOW, FONT_24PT);

    if(wifi_init() == PICO_ERROR_NONE){
        if(wifi_connect(SSID, PASSWD) == PICO_ERROR_NONE){
            debug_info("Connected to Wifi\n");
        } else{
            debug_err("Failed to Connect to Wifi\n");
        }
    }

    ui_draw_temperature_screen(10,18,15);
    sync_rtc();
    
    uint16_t temp = 0;
    uint16_t temp2 = 50;
    uint16_t temp3 = 25;
    while (1){
        ui_update_temperature(TEMP_L1, temp++);
        ui_update_temperature(TEMP_L2, temp2++);
        ui_update_temperature(TEMP_L3, temp3++);
        ui_display_time();
        if(temp > 600) temp = 0;
        if(temp2 > 600) temp2 = 0;
        if(temp3 > 600) temp3 = 0;
        debug_pin_on();
        sleep_ms(1000); 
    }
}
