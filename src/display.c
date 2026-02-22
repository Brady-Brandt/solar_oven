#include "pico/stdlib.h"
#include "display.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"



#define CLK 2
#define LCD_DIN 3
#define DATAOUT 4
#define CS 5
#define DC 6
#define RESET 7

#define DISPLAY_SLEEP_OUT 0x11
#define DISPLAY_ON 0x29
#define DISPLAY_CASET 0x2A
#define DISPLAY_RASET 0x2B
#define DISPLAY_MEM_WRITE 0x2C
#define DISPLAY_MADCTL 0x36
#define DISPLAY_INVERSION 0x21


#define SPI_BAUDRATE 25000000



static void display_reset(){
   gpio_put(RESET, 0);
   sleep_ms(10);
   gpio_put(RESET, 1);
   sleep_ms(150);
}


static void display_send_cmd(uint8_t cmd){
    gpio_put(DC, 0);
    gpio_put(CS, 0);
    spi_write_blocking(spi0,&cmd, 1);
    gpio_put(CS,1);
}


static void display_write_data(uint8_t *data, size_t len) {
    gpio_put(DC, 1);
    gpio_put(CS, 0);
    spi_write_blocking(spi0, data, len);
    gpio_put(CS, 1);
}

static void display_write_byte(uint8_t data) {
    gpio_put(DC, 1);
    gpio_put(CS, 0);
    spi_write_blocking(spi0, &data, 1);
    gpio_put(CS, 1);
}


void display_init(){
    spi_init(spi0,SPI_BAUDRATE);
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);


    gpio_set_function(CLK, GPIO_FUNC_SPI);
    gpio_set_function(LCD_DIN, GPIO_FUNC_SPI);


    gpio_init(CS); 
    gpio_set_dir(CS, GPIO_OUT);
    gpio_init(DC); 
    gpio_set_dir(DC, GPIO_OUT);
    gpio_init(RESET); 
    gpio_set_dir(RESET, GPIO_OUT);

    display_reset();
    display_send_cmd(DISPLAY_SLEEP_OUT);
    sleep_ms(150);

    display_send_cmd(DISPLAY_ON);
    sleep_ms(10);

    display_send_cmd(DISPLAY_INVERSION);
    sleep_ms(10);

    display_send_cmd(DISPLAY_MADCTL);
    display_write_byte(0x28);
    sleep_ms(10);
}



static void set_address_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint8_t data_col[4] = { x0 >> 8, x0 & 0xFF, x1 >> 8, x1 & 0xFF };
    uint8_t data_row[4] = { y0 >> 8, y0 & 0xFF, y1 >> 8, y1 & 0xFF };

    display_send_cmd(DISPLAY_CASET);
    display_write_data(data_col, 4);

    display_send_cmd(DISPLAY_RASET);
    display_write_data(data_row, 4);
}



void display_background_color(uint8_t r, uint8_t g, uint8_t b){ 
    RGB666 color = RGB_TO_RGB666(r, g, b);
    set_address_window(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);
    display_send_cmd(DISPLAY_MEM_WRITE);

    gpio_put(DC, 1);
    gpio_put(CS, 0);

    for(uint32_t i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
        spi_write_blocking(spi0, color, 3);
    }


    

    gpio_put(CS, 1);


}

