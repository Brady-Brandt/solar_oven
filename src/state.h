#pragma once
#include <stdint.h>
#include "wifi.h"


#define TIME_SYNC_IN_PROGESS 0x02
#define TIME_SYNCED 0x01

typedef struct {
    float temperature;
    float sensor1;
    float sensor2;
    float sensor3;
    uint16_t timer;
    WifiStatus wifi_status;
    uint8_t time_synced;
    int8_t utc_offset;
    uint8_t is_celsius;
} ProgramState;


extern ProgramState program_state;

// store time paused in msb of timer
#define TIMER_MASK 0x7fff
#define time_get()       (program_state.timer & TIMER_MASK)
#define time_is_paused() (program_state.timer >> 15)
#define time_unpause()   (program_state.timer ^= (1 << 15))
#define time_is_up()    ((program_state.timer & TIMER_MASK) == 0)
