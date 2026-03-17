#include "ui.h"
#include "font.h"
#include "display.h"
#include <hardware/rtc.h>
#include <pico/time.h>
#include <pico/types.h>
#include <stdint.h>


static uint8_t labels_len = 0;
#define MAX_LABELS 16
static Label labels[MAX_LABELS];

//temperature labels
uint8_t TEMP_L1;
uint8_t TEMP_L2;
uint8_t TEMP_L3;

//temperature as a string for each sensor
static char TEMP1[4] = {0};
static char TEMP2[4] = {0};
static char TEMP3[4] = {0};

//Radius of the Degrees Symbol
#define DEGREES_RAD 3

uint8_t create_label(uint16_t x, uint16_t y, uint16_t fg, uint16_t bg, char* txt, FontSize font_size){
    if(labels_len == MAX_LABELS - 1) labels_len = 0;

    uint8_t lbl = labels_len;
    
    labels[labels_len].x = x;
    labels[labels_len].y = y;
    labels[labels_len].fg = fg;
    labels[labels_len].bg = bg;
    labels[labels_len].txt = txt;
    labels[labels_len].font = font_size;
    display_get_text_size(txt, font_size, &labels[labels_len].w, &labels[labels_len].h, &labels[labels_len].lower_bound);
    labels_len++;

    return lbl;
}

void ui_clear(uint16_t bg){
    for (int i = 0; i < labels_len; i++) {
        Label l = labels[i];
        display_draw_box(l.x, l.y - l.h, l.w, l.h + l.lower_bound, bg);
    }
    labels_len = 0;
}

static inline void num_to_string(uint16_t num, char str[4]){
    str[0] = (num / 100) + '0';
    str[1] = ((num / 10) % 10) + '0';
    str[2] = (num % 10) + '0';
}


void ui_draw_temperature_screen(uint16_t s1, uint16_t s2, uint16_t s3){
    uint8_t padding = 25;
    uint8_t l1 = create_label(20, 100, NDSU_YELLOW, NO_BACKGROUND_COLOR, "Temp Sensor 1:", FONT_12PT);

    uint16_t xoffset = padding + labels[l1].w;

    num_to_string(s1, TEMP1);
    num_to_string(s2, TEMP2);
    num_to_string(s3, TEMP3);

    TEMP_L1 = create_label(xoffset, 100, NDSU_YELLOW, NO_BACKGROUND_COLOR, TEMP1, FONT_12PT);
    display_draw_circle(labels[TEMP_L1].x + labels[TEMP_L1].w + DEGREES_RAD, 
            labels[TEMP_L1].y - labels[TEMP_L1].h + DEGREES_RAD, DEGREES_RAD, NDSU_YELLOW);
    labels[TEMP_L1].w += DEGREES_RAD * 3;

    create_label(20, 140, NDSU_YELLOW, NO_BACKGROUND_COLOR, "Temp Sensor 2:", FONT_12PT);
    TEMP_L2 = create_label(xoffset, 140, NDSU_YELLOW, NO_BACKGROUND_COLOR, TEMP2, FONT_12PT);
    display_draw_circle(labels[TEMP_L2].x + labels[TEMP_L2].w + DEGREES_RAD, 
            labels[TEMP_L2].y - labels[TEMP_L2].h + DEGREES_RAD, DEGREES_RAD, NDSU_YELLOW);

    labels[TEMP_L2].w += DEGREES_RAD * 3;


    create_label(20, 180, NDSU_YELLOW, NO_BACKGROUND_COLOR, 
            "Temp Sensor 3:", FONT_12PT);

    TEMP_L3 = create_label(xoffset, 180, NDSU_YELLOW, NO_BACKGROUND_COLOR, TEMP3, FONT_12PT);
    display_draw_circle(labels[TEMP_L3].x + labels[TEMP_L3].w + DEGREES_RAD, 
            labels[TEMP_L3].y - labels[TEMP_L3].h + DEGREES_RAD, DEGREES_RAD, NDSU_YELLOW);
    labels[TEMP_L3].w += DEGREES_RAD * 3;


    

    for (int i = 0; i < labels_len; i++) {
        Label l = labels[i];
        display_draw_text(l.txt, l.x, l.y, l.fg, l.font); 
    }

}

void ui_update_temperature(uint8_t lbl, uint16_t temp){
    num_to_string(temp, labels[lbl].txt);
    Label l1 = labels[lbl];
    //clear out old temperature
    display_draw_box(l1.x, l1.y - l1.h, l1.w, l1.h + l1.lower_bound, NDSU_GREEN);

    //get new size
    display_get_text_size(labels[lbl].txt, labels[lbl].font, &labels[lbl].w, &labels[lbl].h, &labels[lbl].lower_bound);


    //draw new temperature
    display_draw_text(labels[lbl].txt, labels[lbl].x, labels[lbl].y, labels[lbl].fg, labels[lbl].font); 
    display_draw_circle(labels[lbl].x + labels[lbl].w + DEGREES_RAD, 
            labels[lbl].y - labels[lbl].h + DEGREES_RAD, DEGREES_RAD, NDSU_YELLOW);
    labels[lbl].w += DEGREES_RAD * 3;
}


#define TIME_X 10
#define TIME_Y 305
#define TIME_H 15

void ui_display_time(){
    //clear out old temperature
    display_draw_box(TIME_X, TIME_Y - TIME_H, 90, TIME_H + 5, NDSU_GREEN);

    datetime_t t;
    rtc_get_datetime(&t);
    // convert from utc to current time zone
    // maybe include an option to set time zone but just going to hardcode for now
    t.hour -= 5;
    if(t.hour < 0){
        t.hour += 24;
    }

    char apm = 'a';
    if(t.hour >= 12){
       apm = 'p'; 
    }
    uint16_t hour = (t.hour == 0) ? 12 : t.hour; 
    hour = (hour > 12) ? hour - 12 : hour;
 
    char txt[9];    
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
}
