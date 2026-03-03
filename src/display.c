#include "pico/stdlib.h"
#include "display.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include <stdint.h>
#include "font.h"
#include "FreeMono24pt7b.h"



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

#define BUFFER_SIZE 128 * 32
static uint8_t BUFFER[BUFFER_SIZE];
static uint16_t buffer_len = 0;


static void display_reset(){
   gpio_put(RESET, 0);
   sleep_ms(10);
   gpio_put(RESET, 1);
   sleep_ms(150);
}


static void send_cmd(uint8_t cmd){
    gpio_put(DC, 0);
    gpio_put(CS, 0);
    spi_write_blocking(spi0,&cmd, 1);
    gpio_put(CS,1);
}

//starts a write command and opens stream for data
static inline void start_write(){
    gpio_put(CS, 0);
    gpio_put(DC, 0);
    const uint8_t WRITE = DISPLAY_MEM_WRITE;
    spi_write_blocking(spi0, &WRITE, 1);
    gpio_put(DC, 1);
}

static inline void end_write(){
    gpio_put(CS, 1);
}

//sends a command along with its parameters
static void send_cmd_with_data(uint8_t cmd, uint8_t* data, size_t len){
    gpio_put(DC, 0);
    gpio_put(CS, 0);
    spi_write_blocking(spi0,&cmd, 1);
    gpio_put(DC, 1);
    spi_write_blocking(spi0, data, len);
    gpio_put(CS, 1);
}


static void write_byte(uint8_t data) {
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
    send_cmd(DISPLAY_SLEEP_OUT);
    sleep_ms(150);

    send_cmd(DISPLAY_ON);
    sleep_ms(10);

    send_cmd(DISPLAY_INVERSION);
    sleep_ms(10);

    send_cmd(DISPLAY_MADCTL);
    write_byte(0x28);
    sleep_ms(10);
}



static void set_address_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    uint8_t data_col[4] = { x0 >> 8, x0 & 0xFF, x1 >> 8, x1 & 0xFF };
    uint8_t data_row[4] = { y0 >> 8, y0 & 0xFF, y1 >> 8, y1 & 0xFF };

    send_cmd_with_data(DISPLAY_CASET, data_col, 4);
    send_cmd_with_data(DISPLAY_RASET, data_row, 4);
}



void display_background_color(uint8_t r, uint8_t g, uint8_t b){ 
    RGB666 color = RGB_TO_RGB666(r, g, b);
    set_address_window(0, 0, DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1);

    start_write();

    buffer_len = 0;
    for(uint32_t i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
        BUFFER[buffer_len++] = color[0];
        BUFFER[buffer_len++] = color[1];
        BUFFER[buffer_len++] = color[2];
        if(buffer_len >= BUFFER_SIZE - 3){
            spi_write_blocking(spi0, BUFFER, buffer_len);
            buffer_len = 0;
        }
    }
    if(buffer_len != 0) spi_write_blocking(spi0, BUFFER, buffer_len);
    end_write();
}


void display_draw_box(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t* color){
    if(w == 0 || h == 0) return;

    set_address_window(x,y,x + w - 1,y + h - 1);

    start_write(); 

    uint32_t pixels = (uint32_t)w * h;

    buffer_len = 0;
    for(uint32_t i = 0; i < pixels; i++) {
        BUFFER[buffer_len++] = color[0];
        BUFFER[buffer_len++] = color[1];
        BUFFER[buffer_len++] = color[2];
        if(buffer_len >= BUFFER_SIZE - 3){
            spi_write_blocking(spi0, BUFFER, buffer_len);
            buffer_len = 0;
        }
    }
    if(buffer_len != 0) spi_write_blocking(spi0, BUFFER, buffer_len); 
    end_write();
}




void display_draw_text(const char* text, uint16_t x, uint16_t y){
    uint16_t cursor_x = x;
    uint8_t color[3] = {NDSU_YELLOW};

    while(*text){
        char c = *text++;
        GFXglyph glyph = FreeMono24pt7b.glyph[c - FreeMono24pt7b.first];
        uint8_t *bitmap = FreeMono24pt7b.bitmap;

        uint16_t bitmap_offset = glyph.bitmapOffset;
        uint8_t current_byte = 0, bit = 0;

        for (uint8_t row = 0; row < glyph.height; row++) {
            for (uint8_t col = 0; col < glyph.width; col++) {
                if (!(bit++ & 7)) {
                    current_byte = bitmap[bitmap_offset++];
                }
                if (current_byte & 0x80) {
                    // TODO OPTIMIZE THIS
                    display_draw_box(x + (glyph.xOffset + col), y + (glyph.yOffset + row),
                            1, 1, color);
                }
                current_byte <<= 1;
            }
        }
        x += glyph.xAdvance;
    }
}
