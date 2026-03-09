#include <pico/time.h>
#include <stdint.h>
#include "display.h"
#include "font.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

const uint LED_PIN = 16;

int main() {

    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    display_init();

    display_background_color(NDSU_GREEN);
 
    uint16_t yellow = NDSU_YELLOW;

    display_draw_text("Solar Oven", 100, 50, yellow, FONT_24PT);
    display_draw_text("Temp Sensor 1:", 20, 100, yellow, FONT_12PT);
    display_draw_text("Temp Sensor 2:", 20, 140, yellow, FONT_12PT);
    display_draw_text("Temp Sensor 3:", 20, 180, yellow, FONT_12PT);
    

    while (1){ 
        gpio_put(LED_PIN, 1);
        sleep_ms(5000);
    }
}
