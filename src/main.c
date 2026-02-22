#include <pico/time.h>
#include <stdint.h>
#include <stdio.h>
#include "display.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

const uint LED_PIN = 16;


typedef uint8_t RGB666[3];

#define RGB_TO_RGB666(red,green,blue) {(red * 63/ 255) << 2, (green * 63/ 255) << 2, (blue * 63/ 255) << 2} 


int main() {

    stdio_init_all();
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    display_init();

     

    RGB666 color = RGB_TO_RGB666(0, 0, 0);
    


    display_background_color(255,0,0);
    


    while (1) {
        for (int i = 0; i < 3; i++) {
            printf("%d\n", color[i]);
        
        }
        gpio_put(LED_PIN, 1);
        sleep_ms(1000);
    }
}
