#include "touchscreen.h"
#include "display.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "pins.h"
#include <pico/time.h>
#include <stdint.h>


#define ADDR          0x38
#define I2C_BUS       i2c_default
// -----------------------------------------------------------------------------
// FT6336 registers
// -----------------------------------------------------------------------------
#define FT_DEVICE_MODE        0x00  // Device/mode control register
#define FT_REG_NUM_FINGER     0x02  // Number of touch points

#define FT_TP1_REG            0x03  // First touch-point data address
#define FT_TP2_REG            0x09  // Second touch-point data address

#define FT_ID_G_CIPHER_MID    0x9F  // Chip ID, middle byte; default 0x26
#define FT_ID_G_CIPHER_LOW    0xA0  // Chip ID, low byte
                                    // 0x01 = FT6336G
                                    // 0x02 = FT6336U

#define FT_ID_G_LIB_VERSION   0xA1  // Library/version

#define FT_ID_G_CIPHER_HIGH   0xA3  // Chip ID, high byte; default 0x64

#define FT_ID_G_MODE          0xA4  // Interrupt-mode control register

#define FT_ID_G_FOCALTECH_ID  0xA8  // FocalTech vendor ID; default 0x11

#define FT_ID_G_THGROUP       0x80  // Touch threshold setting

#define FT_ID_G_PERIODACTIVE  0x88  // Active-period setting

/*
static void write_reg(uint8_t reg, uint8_t data){
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = data;
    i2c_write_blocking(I2C_BUS, ADDR, buf, 2, false);
}
*/

static void read(uint8_t reg, uint8_t* data, size_t len){
    i2c_write_blocking(I2C_BUS, ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_BUS, ADDR, data, len, false);
}

void touchscreen_init(){
    i2c_init(I2C_BUS, 100000);    
    gpio_set_function(PIN_TOUCH_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_TOUCH_SCL, GPIO_FUNC_I2C);

    gpio_init(PIN_TOUCH_INT);
    gpio_set_dir(PIN_TOUCH_INT, GPIO_OUT);
    gpio_init(PIN_TOUCH_RESET);
    gpio_pull_up(PIN_TOUCH_RESET);
    gpio_set_dir(PIN_TOUCH_RESET, GPIO_OUT);

    gpio_put(PIN_TOUCH_RESET,0);
    gpio_put(PIN_TOUCH_INT,0);
    sleep_us(1000);
    gpio_put(PIN_TOUCH_RESET, 1);
    sleep_ms(110);
    gpio_set_dir(PIN_TOUCH_INT, GPIO_IN);
}


bool touchscreen_get_point(uint16_t* x, uint16_t* y){ 
    uint8_t points = 0;
    // Read number of active touch points
    read(FT_REG_NUM_FINGER, &points, 1);
    if(points){
        //00 = Touch Down
        //01 = Lift Up
        //10 = Contact / still touching
        //11 = reserved
        uint8_t buff[4];
        read(FT_TP1_REG,buff,4);

        // seems to default to portrait mode. I need landscape
        *y = DISPLAY_HEIGHT - (((0xF & buff[0]) << 8) | buff[1]);
        *x = ((0xF & buff[2]) << 8) | buff[3];
        return true;
    }
    *x = 0;
    *y = 0;
    return false;
}
