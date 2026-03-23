#include "ui.h"
#include "font.h"
#include "display.h"
#include "state.h"
#include "wifi.h"
#include <hardware/rtc.h>
#include <pico/time.h>
#include <pico/types.h>
#include <stdint.h>


static uint8_t bounding_len = 0;
#define MAX_BOUNDING_BOXES 16
static BoundingBox boxes[MAX_BOUNDING_BOXES];

//Radius of the Degrees Symbol
#define DEGREES_RAD 3

static void bounding_box_create_text(uint8_t idx, uint16_t x, uint16_t y, char* txt, FontSize font_size){
    boxes[idx].x = x;
    boxes[idx].y = y;
    display_get_text_size(txt, font_size,&boxes[idx].w,&boxes[idx].h, &boxes[idx].lower_bound);
    bounding_len++;
}

#define bounding_box_update_text(idx, txt, font_size) \
    display_get_text_size(txt, font_size, &boxes[idx].w,&boxes[idx].h, &boxes[idx].lower_bound)

#define bounding_box_draw_text(idx, txt, color, font_size) \
    display_draw_text(txt, boxes[idx].x, boxes[idx].y, color, font_size)

#define bounding_box_clear_text(idx, bg) \
    display_draw_box(boxes[idx].x, boxes[idx].y - boxes[idx].h, boxes[idx].w, boxes[idx].h + boxes[idx].lower_bound, bg);

#define bounding_box_draw_degrees_symbol(idx, rad, color) \
    do { \
        display_draw_circle(boxes[idx].x + boxes[idx].w + rad, boxes[idx].y - boxes[idx].h + rad, rad, color); \
        boxes[idx].w += rad * 3; \
    } while(0)



void ui_clear(uint16_t bg){
    for (int i = 0; i < bounding_len; i++) {
        BoundingBox l = boxes[i];
        display_draw_box(l.x, l.y - l.h, l.w, l.h + l.lower_bound, bg);
    }
    bounding_len = 0;
}

static inline void num_to_string(uint16_t num, char str[4]){
    str[0] = (num / 100) + '0';
    str[1] = ((num / 10) % 10) + '0';
    str[2] = (num % 10) + '0';
    str[3] = 0;
}

#define S1_IDX 3
#define S2_IDX 4
#define S3_IDX 5
void ui_draw_temperature_full() {
    uint8_t padding = 25;

    char s1_str[4],s2_str[4],s3_str[4];

    num_to_string(program_state.sensor1, s1_str);
    num_to_string(program_state.sensor2, s2_str);
    num_to_string(program_state.sensor3, s3_str);

    //this branch is almost always taken
    if(__builtin_expect(bounding_len != 0, 1)){
        //clear out old temperature/time
        bounding_box_clear_text(S1_IDX, NDSU_GREEN);
        bounding_box_clear_text(S2_IDX, NDSU_GREEN);
        bounding_box_clear_text(S3_IDX, NDSU_GREEN);
        //update new bounding boxes
        bounding_box_update_text(S1_IDX, s1_str, FONT_12PT);
        bounding_box_update_text(S2_IDX, s2_str, FONT_12PT);
        bounding_box_update_text(S3_IDX, s3_str, FONT_12PT);
    } else{
        bounding_box_create_text(0, 20, 100, "Temp Sensor 1:", FONT_12PT);
        bounding_box_create_text(1, 20, 140, "Temp Sensor 2:", FONT_12PT);
        bounding_box_create_text(2, 20, 180, "Temp Sensor 3:", FONT_12PT);

        bounding_box_draw_text(0, "Temp Sensor 1:", NDSU_YELLOW, FONT_12PT);
        bounding_box_draw_text(1, "Temp Sensor 2:", NDSU_YELLOW, FONT_12PT);
        bounding_box_draw_text(2, "Temp Sensor 3:", NDSU_YELLOW, FONT_12PT);


        uint16_t xoffset = padding + boxes[0].w;
        bounding_box_create_text(S1_IDX, xoffset, 100, s1_str, FONT_12PT);
        bounding_box_create_text(S2_IDX, xoffset, 140, s2_str, FONT_12PT);
        bounding_box_create_text(S3_IDX, xoffset, 180, s3_str, FONT_12PT);
    }


    bounding_box_draw_text(S1_IDX, s1_str, NDSU_YELLOW, FONT_12PT);
    bounding_box_draw_degrees_symbol(S1_IDX, DEGREES_RAD, NDSU_YELLOW);

    bounding_box_draw_text(S2_IDX, s2_str, NDSU_YELLOW, FONT_12PT);
    bounding_box_draw_degrees_symbol(S2_IDX, DEGREES_RAD, NDSU_YELLOW);

    bounding_box_draw_text(S3_IDX, s3_str, NDSU_YELLOW, FONT_12PT);
    bounding_box_draw_degrees_symbol(S3_IDX, DEGREES_RAD, NDSU_YELLOW);
}

#define TIME_X 385
#define TIME_Y TASKBAR_Y
#define TIME_H 15

void ui_display_time(){
    //clear out old temperature
    display_draw_box(TIME_X, TIME_Y - TIME_H, 90, TIME_H + 5, NDSU_GREEN);

    if(program_state.time_synced == TIME_SYNCED){
        char txt[9];
        datetime_t t;
        rtc_get_datetime(&t);
        t.hour += program_state.utc_offset;
        if(t.hour < 0){
            t.hour += 24;
        }
        char apm = 'a';
        if(t.hour >= 12){
           apm = 'p';
        }
        uint16_t hour = (t.hour == 0) ? 12 : t.hour;
        hour = (hour > 12) ? hour - 12 : hour;

        txt[0] = ((hour / 10) % 10) + '0';
        txt[1] = (hour % 10) + '0';
        txt[2] = ':';
        txt[3] = ((t.min / 10) % 10) + '0';
        txt[4] = (t.min% 10) + '0';
        txt[5] = ' ';
        txt[6] = apm;
        txt[7] = 'm';
        txt[8] = 0;
        display_draw_text(txt, TIME_X, TIME_Y, NDSU_YELLOW, FONT_9PT);
    }else if(program_state.time_synced == 0) {
        char txt[9] = "Time Err";
        display_draw_text(txt, TIME_X, TIME_Y, RGB565(255,0,0), FONT_9PT);
    }

}


#define WIFI_X 60
#define WIFI_W 110
#define WIFI_H 15
void ui_display_wifi_status(){
    display_draw_box(WIFI_X, TASKBAR_Y- WIFI_H, WIFI_W, WIFI_H + 1, NDSU_GREEN);
    switch(wifi_status()){
        case WIFI_DOWN:
            display_draw_text("Off", WIFI_X, TASKBAR_Y, RGB565(192,192,196), FONT_9PT);
            break;
        case WIFI_CONNECTED:
            display_draw_text("Connected", WIFI_X, TASKBAR_Y, RGB565(0,255,0), FONT_9PT);
            break;
        case WIFI_FAILED:
            display_draw_text("Failed", WIFI_X, TASKBAR_Y, RGB565(255,0,0), FONT_9PT);
            break;
        case WIFI_NONET:
            display_draw_text("No Network", WIFI_X, TASKBAR_Y, RGB565(255,165,0), FONT_9PT);
            break;
        case WIFI_BADAUTH:
            display_draw_text("Auth Error", WIFI_X, TASKBAR_Y, RGB565(0,255,255), FONT_9PT);
            break;

    }
}


#define TEMP_IDX 0
#define TIMER_IDX 1


void ui_draw_timer_and_temp() {
    uint16_t temp = (uint16_t)((program_state.sensor1 + program_state.sensor1 + program_state.sensor3) / 3.0);
    char temp_str[4];
    num_to_string(temp, temp_str);

    int minutes = program_state.timer / 60;
    int seconds = program_state.timer % 60;

    char time_str[6];
    time_str[0] = '0' + (minutes / 10);
    time_str[1] = '0' + (minutes % 10);
    time_str[2] = ':';
    time_str[3] = '0' + (seconds / 10);
    time_str[4] = '0' + (seconds % 10);
    time_str[5] = '\0';

    //this branch is almost always taken
    if(__builtin_expect(bounding_len != 0, 1)){
        //clear out old temperature/time
        bounding_box_clear_text(TEMP_IDX, NDSU_GREEN);
        bounding_box_clear_text(TIMER_IDX, NDSU_GREEN);
        //update new bounding boxes
        bounding_box_update_text(TEMP_IDX, temp_str, FONT_24PT);
        bounding_box_update_text(TIMER_IDX, time_str, FONT_24PT);
    } else{
        bounding_box_create_text(TEMP_IDX, 200, 120, temp_str, FONT_24PT);
        bounding_box_create_text(TIMER_IDX, 170, 190, time_str, FONT_24PT);
    }
    bounding_box_draw_text(TEMP_IDX, temp_str, NDSU_YELLOW, FONT_24PT);
    bounding_box_draw_degrees_symbol(TEMP_IDX, (DEGREES_RAD + 1), NDSU_YELLOW);
    bounding_box_draw_text(TIMER_IDX, time_str, NDSU_YELLOW, FONT_24PT);
}
