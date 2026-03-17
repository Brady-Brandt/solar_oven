#include <pico/time.h>
#include <pico/types.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "display.h"
#include "font.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/rtc.h"
#include "wifi.h"
#include "ui.h"

const uint LED_PIN = 16;

int main() {
    stdio_init_all();
    rtc_init();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    wifi_init(); 
    wifi_connect(SSID, PASSWD);
    display_init();
    display_background_color(NDSU_GREEN);
 
    uint16_t yellow = NDSU_YELLOW;

    display_draw_text("Solar Oven", 100, 50, yellow, FONT_24PT);
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
        gpio_put(LED_PIN, 1);
        sleep_ms(1000); 
    }
}
