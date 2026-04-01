#include <pico/time.h>
#include <pico/types.h>
#include <stdbool.h>
#include <stdint.h>
#include "display.h"
#include "font.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/rtc.h"
#include "hardware/adc.h"
#include "debug.h"
#include "sensors.h"
#include "wifi.h"
#include "ui.h"
#include "state.h"

ProgramState program_state = {0};


static bool update_time(repeating_timer_t *rt){
    if(time_is_up()){
        gpio_xor_mask(1 << 13);
        return true;
    } else if (time_is_paused()) {
        return true;
    }
    program_state.timer--;
    return true;
}

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

    sensors_init();

    display_draw_text("WIFI: ", 5, TASKBAR_Y, NDSU_YELLOW, FONT_9PT);
    sync_rtc();
    
    program_state.sensor1 = 0;
    program_state.sensor2 = 50;
    program_state.sensor3 = 50;
    program_state.timer = 60;

    /*
    gpio_init(13);
    gpio_set_dir(13, GPIO_OUT);
    gpio_put(13, 0);

    repeating_timer_t timer;
    add_repeating_timer_ms(1000, update_time, NULL, &timer);
    */
    while (1){
        debug_info("Monostable: %f\n",program_state.sensor1);
        debug_info("ADC       : %f\n",program_state.sensor2);
        ui_display_time();
        ui_draw_timer_and_temp();

        program_state.sensor3++;
        //TODO: THIS DOESN'T NEED TO BE CALLED THIS OFTEN
        ui_display_wifi_status();
        debug_pin_on();
        sleep_ms(1000);
    }
}
