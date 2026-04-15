#include "touchscreen.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include <pico/time.h>
#include <stdint.h>

#define SDA_PIN        8
#define SCL_PIN        9
#define RESET_PIN     10
#define INTERRUPT_PIN 11
#define ADDR          0x5D

#define _COMMAND          0x8040
#define _CONFIG_VERSION   0x8047
#define _RESOLUTION_X     0x8048
#define _RESOLUTION_Y     0x804A
#define _TOUCH_POINTS     0x804C
#define _MODULE_SWITCH1   0x804D
#define _REFRESH_RATE     0x8056
#define _CONFIG_CHKSUM    0x80FF
#define _CONFIG_FRESH     0x8100

#define _DATA_BUFFER      0x814E
#define _POINT_DATA_START 0x8150

#define I2C_BUS i2c_default

static void write_reg16_8(uint16_t reg, uint8_t data)
{
    uint8_t buf[3];

    buf[0] = (reg >> 8) & 0xFF;  // reg high byte
    buf[1] = reg & 0xFF;         // reg low byte
    buf[2] = data;               // 1-byte value

    i2c_write_blocking(I2C_BUS, ADDR, buf, 3, false);
}


static void write_reg16_16(uint16_t reg, uint16_t data)
{
    uint8_t buf[4];

    buf[0] = (reg >> 8) & 0xFF;
    buf[1] = reg & 0xFF;

    buf[2] = data & 0xFF;        // low byte first (little endian)
    buf[3] = (data >> 8) & 0xFF; // high byte

    i2c_write_blocking(I2C_BUS, ADDR, buf, 4, false);
}


static void read(uint16_t reg, uint8_t* data, size_t len){
    uint8_t regbuf[2];

    regbuf[0] = (reg >> 8) & 0xFF;
    regbuf[1] = reg & 0xFF;

    i2c_write_blocking(I2C_BUS, ADDR, regbuf, 2, true);
    i2c_read_blocking(I2C_BUS, ADDR, data, len, false);
}

void touchscreen_init(){
    i2c_init(I2C_BUS, 100000);    
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);

    gpio_init(INTERRUPT_PIN);
    gpio_set_dir(INTERRUPT_PIN, GPIO_OUT);
    gpio_init(RESET_PIN);
    gpio_set_dir(RESET_PIN, GPIO_OUT);

    gpio_put(RESET_PIN,0);
    gpio_put(INTERRUPT_PIN,0);
    sleep_us(1000);
    gpio_put(RESET_PIN, 1);
    sleep_ms(110);

    gpio_set_dir(INTERRUPT_PIN, GPIO_IN);

    write_reg16_16(_RESOLUTION_X, 480);
    write_reg16_16(_RESOLUTION_Y, 320);
    write_reg16_8(_TOUCH_POINTS, 1);

    uint8_t value = 0 << 7 | 1 << 6 | 1 << 3 | 1 << 2 | 1; 
    write_reg16_8(_MODULE_SWITCH1, value);
    write_reg16_8(_REFRESH_RATE, 50);

    write_reg16_8(_COMMAND, 0x00);

    uint8_t config[184];
    read(_CONFIG_VERSION, config, 184);
    uint8_t sum = 0;
    for(int i = 0; i < 184; i++){
        sum += config[i];
    }
    sum = ~sum + 1;
    write_reg16_8(_CONFIG_CHKSUM, sum);
    write_reg16_8(_CONFIG_FRESH, 1);
}



bool touchscreen_get_point(uint16_t* x, uint16_t* y){ 
    uint8_t status = 0;
    read(_DATA_BUFFER, &status, 1);
    if(status & 0x80){
        uint8_t num_points = status & 0x0F;
        if(num_points == 0){
            write_reg16_8(_DATA_BUFFER, 0);
            goto no_touches;
        }
        
        uint8_t x_res[2], y_res[2];
        read(_POINT_DATA_START, x_res, 2);
        read(_POINT_DATA_START + 2, y_res, 2);

        *x = (x_res[0] | (x_res[1] << 8));
        *y = (y_res[0] | (y_res[1] << 8));
        write_reg16_8(_DATA_BUFFER, 0);
        return true;
    }

no_touches:
    *x = 0;
    *y = 0;
    return false;
}
