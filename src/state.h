#pragma once
#include <stdint.h>
#include "wifi.h"


#define TIME_SYNC_IN_PROGESS 0x02
#define TIME_SYNCED 0x01

typedef struct {
    uint16_t temperature;
    volatile float sensor1;
    volatile float sensor2;
    volatile uint16_t timer;
    WifiStatus wifi_status;
    uint8_t screen;
    uint8_t prev_screen;
    volatile uint8_t time_synced;
    int8_t utc_offset;
    uint8_t is_celsius;
} ProgramState;

#define HOME_SCREEN        0
#define SETTINGS_SCREEN    1
#define DIAGNOSTICS_SCREEN 2


extern ProgramState program_state;

// store time paused in msb of timer
#define TIMER_MASK 0x7fff
#define time_get()       (program_state.timer & TIMER_MASK)
#define time_is_paused() (program_state.timer >> 15)
#define time_unpause()   (program_state.timer ^= (1 << 15))
#define time_is_up()    ((program_state.timer & TIMER_MASK) == 0)


/*
 * @brief Writes data from program state to flash
 *
 *
 */
void save_data_to_flash();

/*
 * @brief Sets the program state to values in the flash if valid
 *
 *
 */
void get_data_from_flash();
