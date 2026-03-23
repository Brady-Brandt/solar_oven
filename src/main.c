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
#include "state.h"

ProgramState program_state = {0};

int main() {
    program_state.utc_offset = -5;
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

    display_draw_text("WIFI: ", 5, TASKBAR_Y, NDSU_YELLOW, FONT_9PT);
    sync_rtc();
    
    program_state.sensor1 = 0;
    program_state.sensor2 = 50;
    program_state.sensor3 = 50;
    while (1){
        ui_display_time();
        ui_draw_timer_and_temp();
        program_state.sensor1++;
        program_state.sensor2++;
        program_state.sensor3++;
        program_state.timer++;
        //TODO: THIS DOESN'T NEED TO BE CALLED THIS OFTEN
        ui_display_wifi_status();
        debug_pin_on();
        sleep_ms(1000);
    }
}
