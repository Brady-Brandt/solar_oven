#include <pico/time.h>
#include <stdint.h>
#include "display.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

const uint LED_PIN = 16;

int main() {

    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    display_init();

    display_background_color(NDSU_GREEN);
 
    RGB666 c = {NDSU_YELLOW};
    display_draw_box(200, 200,100, 40, c);

    display_draw_text("Hello World", 100, 100);
    

    while (1){ 
        gpio_put(LED_PIN, 1);
        sleep_ms(5000);
    }
}
