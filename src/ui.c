#include "ui.h"
#include "https.h"
#include "debug.h"
#include "font.h"
#include "display.h"
#include "state.h"
#include "wifi.h"
#include "touchscreen.h"
#include <hardware/gpio.h>
#include <hardware/rtc.h>
#include <pico/time.h>
#include <pico/types.h>
#include <stdint.h>
#include <math.h>


static uint8_t bounding_len = 0;
#define MAX_BOUNDING_BOXES 16
static BoundingBox boxes[MAX_BOUNDING_BOXES];

#define MAX_BUTTONS 8
static Button buttons[MAX_BUTTONS];
static volatile uint8_t btns_len = 0;


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

#define bounding_box_update(idx,_x,_y,_w,_h,lb) \
    do { \
        boxes[idx].x = _x; \
        boxes[idx].y = _y; \
        boxes[idx].w = _w; \
        boxes[idx].h = _h; \
        boxes[idx].lower_bound = lb; \
    }while(0)

#define btn_create(bb_idx, func) \
    do { \
        buttons[btns_len].callback     = func; \
        buttons[btns_len].last_clicked = to_ms_since_boot(get_absolute_time()); \
        buttons[btns_len].is_pressed   = 0; \
        buttons[btns_len].last_seen    = 0; \
        buttons[btns_len++].bnd_idx    = bb_idx; \
    } while(0)


static bool clear_ui = false;
void ui_clear(uint16_t bg){
    for (int i = 0; i < bounding_len; i++) {
        // button bounding boxes need to be cleared differently than text bbs
        for(int j = 0; j < btns_len; j++){
            Button b = buttons[j];
            if(b.bnd_idx == i){
                BoundingBox l = boxes[i];
                display_draw_box(l.x, l.y, l.w, l.h , bg);
                goto next_loop;
            }
        }
        BoundingBox l = boxes[i];
        display_draw_box(l.x, l.y - l.h, l.w, l.h + l.lower_bound, bg);
next_loop:
        continue;
    }
    bounding_len = 0;
    btns_len = 0;
    clear_ui = true;
}

static inline void num_to_string(uint16_t num, char str[4]){
    str[0] = (num / 100) + '0';
    str[1] = ((num / 10) % 10) + '0';
    str[2] = (num % 10) + '0';
    str[3] = 0;
}

#define S1_IDX 3
#define S2_IDX 4
void ui_draw_temperature_full() {
    uint8_t padding = 25;

    char s1_str[4],s2_str[4];

    num_to_string(program_state.sensor1, s1_str);
    num_to_string(program_state.sensor2, s2_str);

    //this branch is almost always taken
    if(__builtin_expect(bounding_len != 0, 1)){
        //clear out old temperature/time
        bounding_box_clear_text(S1_IDX, NDSU_GREEN);
        bounding_box_clear_text(S2_IDX, NDSU_GREEN);
        //update new bounding boxes
        bounding_box_update_text(S1_IDX, s1_str, FONT_12PT);
        bounding_box_update_text(S2_IDX, s2_str, FONT_12PT);
    } else{
        bounding_box_create_text(0, 20, 100, "Temp Sensor 1:", FONT_12PT);
        bounding_box_create_text(1, 20, 140, "Temp Sensor 2:", FONT_12PT);

        bounding_box_draw_text(0, "Temp Sensor 1:", NDSU_YELLOW, FONT_12PT);
        bounding_box_draw_text(1, "Temp Sensor 2:", NDSU_YELLOW, FONT_12PT);


        uint16_t xoffset = padding + boxes[0].w;
        bounding_box_create_text(S1_IDX, xoffset, 100, s1_str, FONT_12PT);
        bounding_box_create_text(S2_IDX, xoffset, 140, s2_str, FONT_12PT);
    }


    bounding_box_draw_text(S1_IDX, s1_str, NDSU_YELLOW, FONT_12PT);
    bounding_box_draw_degrees_symbol(S1_IDX, DEGREES_RAD, NDSU_YELLOW);

    bounding_box_draw_text(S2_IDX, s2_str, NDSU_YELLOW, FONT_12PT);
    bounding_box_draw_degrees_symbol(S2_IDX, DEGREES_RAD, NDSU_YELLOW); 
}

#define TIME_X 385
#define TIME_Y TASKBAR_Y
#define TIME_H 15

void ui_display_time(){
    static uint8_t prev_min = 255;
    static int8_t prev_utc = 127;

    datetime_t t;
    rtc_get_datetime(&t);

    if(prev_min == t.min && prev_utc == program_state.utc_offset) return;
    prev_min = t.min;
    prev_utc = program_state.utc_offset;

    display_draw_box(TIME_X, TIME_Y - TIME_H, 90, TIME_H + 5, NDSU_GREEN);

    if(program_state.time_synced == TIME_SYNCED){
        char txt[9];
        t.hour = (t.hour + program_state.utc_offset) % 24;
        if (t.hour < 0) {
            t.hour += 24;
        }
        char apm = (t.hour >= 12) ? 'p' : 'a';
        uint16_t hour = t.hour % 12;
        if (hour == 0) {
            hour = 12;
        }

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
    static uint8_t prev_status = 255;

    uint8_t current_status = wifi_status();
    if(current_status == prev_status) return;

    //clear out old status
    display_draw_box(WIFI_X, TASKBAR_Y- WIFI_H, WIFI_W, WIFI_H + 1, NDSU_GREEN);

    switch(current_status){
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
    prev_status = current_status;
}


#define TEMP_IDX   0
#define TIMER_IDX  1
#define PLUS_IDX   2
#define MINUS_IDX  3
#define ON_OFF_IDX 4
#define MORE_IDX   5

void ui_draw_timer_and_temp() {
    static uint16_t prev_temp = 0;
    static uint16_t prev_timer = 0;

    float ftemp = (program_state.sensor1 + program_state.sensor1) / 2.0f;
    //data sent to adafruit will always be in fahrenheit
    program_state.temperature = (uint16_t)roundf(ftemp * 1.8f + 32.0f);
    uint16_t temp = (program_state.is_celsius) ? (uint16_t)roundf(ftemp) : program_state.temperature;
    char temp_str[4];
    num_to_string(temp, temp_str);

    uint16_t current_timer = time_get();
    int minutes = current_timer / 60;
    int seconds = current_timer % 60;

    char time_str[6];
    time_str[0] = '0' + (minutes / 10);
    time_str[1] = '0' + (minutes % 10);
    time_str[2] = ':';
    time_str[3] = '0' + (seconds / 10);
    time_str[4] = '0' + (seconds % 10);
    time_str[5] = '\0';

    //this branch is almost always taken
    if(__builtin_expect(bounding_len != 4, 1)){
        //clear out old temperature/time
        if(prev_timer != current_timer){
            bounding_box_clear_text(TIMER_IDX, NDSU_GREEN);
            bounding_box_update_text(TIMER_IDX, time_str, FONT_24PT);
        }
        if(prev_temp != temp || program_state.prev_screen != program_state.screen){
            bounding_box_clear_text(TEMP_IDX, NDSU_GREEN);
            bounding_box_update_text(TEMP_IDX, temp_str, FONT_24PT);
        }
    } else{
        bounding_box_create_text(TEMP_IDX, 190, 120, temp_str, FONT_24PT);
        bounding_box_create_text(TIMER_IDX, 170, 190, time_str, FONT_24PT);
    }

    if(prev_temp != temp || program_state.prev_screen != program_state.screen){
        bounding_box_draw_text(TEMP_IDX, temp_str, NDSU_YELLOW, FONT_24PT);
        bounding_box_draw_degrees_symbol(TEMP_IDX, (DEGREES_RAD + 1), NDSU_YELLOW);
        display_draw_text((program_state.is_celsius) ? "C" : "F", boxes[TEMP_IDX].x + boxes[TEMP_IDX].w,
            boxes[TEMP_IDX].y,NDSU_YELLOW, FONT_24PT);
        boxes[TEMP_IDX].w += 25; //The size of capital F/C
    }

    bounding_box_draw_text(TIMER_IDX, time_str, NDSU_YELLOW, FONT_24PT);
    prev_timer = current_timer;
    prev_temp = temp;
    // kind of a bandaid solution right now
    program_state.prev_screen = program_state.screen;
}



static void increment_time_cb(ButtonPress press){
    uint16_t time = program_state.timer & TIMER_MASK;
    if(time >= 60*60){
        program_state.timer = (program_state.timer & (1 << 15)) | 60*60;
    } else{
        switch (press) {
            case SHORT_PRESS:
                program_state.timer++;
                break;
            case MEDIUM_PRESS:
                if(time >= 60*60 - 60) program_state.timer = (program_state.timer & (1 << 15)) | 60*60;
                else program_state.timer += 60;
                break;
            case LONG_PRESS:
                if(time >= 60*60 - 180) program_state.timer = (program_state.timer & (1 << 15)) | 60 * 60;
                else program_state.timer += 180;
                break;
        }
    }
}

static void decrement_time_cb(ButtonPress press){
    uint16_t time = program_state.timer & TIMER_MASK;
    if(time == 1){
        program_state.timer = (program_state.timer & (1 << 15)) | 1;
    } else{
        switch (press) {
            case SHORT_PRESS:
                program_state.timer--;
                break;
            case MEDIUM_PRESS:
                if(time <= 60) program_state.timer = (program_state.timer & (1 << 15)) | 1;
                else program_state.timer -= 60;
                break;

            case LONG_PRESS:
                if(time <= 180) program_state.timer = (program_state.timer & (1 << 15)) | 1;
                else program_state.timer -= 180;
                break;
        }
    }
}



static repeating_timer_t ada_timer;
static void start_stop_cb(__unused ButtonPress press){
    // stop the beeper and put 1 minute back on the timer (basically reset)
    if(time_is_up()){
        program_state.timer = (program_state.timer & (1 << 15)) | 60;
        gpio_put(13, 0);
    } else if(time_is_paused()){
        add_repeating_timer_ms(5000,adafruit_send_temperatue,
                NULL, &ada_timer);
        time_unpause();
    } else{
        cancel_repeating_timer(&ada_timer);
        time_unpause();
    }
}


static void more_cb(__unused ButtonPress press){
    program_state.prev_screen = program_state.screen;
    program_state.screen = SETTINGS_SCREEN;
    program_state.timer |= 1 << 15; //stop timer if it is running
    ui_clear(NDSU_GREEN);
    ui_draw_settings_screen();
}


void ui_check_btns(){
    uint16_t x,y;
    bool press = touchscreen_get_point(&x, &y);

    uint32_t now = to_ms_since_boot(get_absolute_time());

    for(int i = 0; i < btns_len; i++){
        // if one of the callbacks trigger a ui clear
        // need to reset checks
        if(clear_ui){
            clear_ui = false;
            break;
        }
        BoundingBox b = boxes[buttons[i].bnd_idx];

        Button *btn = &buttons[i];

        if(press && x >= b.x && x <= b.x + b.w && y >= b.y && y <= b.y + b.h){
            btn->last_seen = now;
            if (!btn->is_pressed && now - btn->last_clicked > 200) {
                // first press
                btn->is_pressed = true;
                btn->press_start = now;
                btn->last_clicked = now;

                btn->callback(SHORT_PRESS);
            }
            else {
                uint32_t held = now - btn->press_start;
                uint32_t since_last = now - btn->last_clicked;

                uint32_t interval = 200;

                if (since_last > interval) {
                    btn->last_clicked = now;
                    if (held > 3000) {
                        btn->callback(LONG_PRESS);
                    }
                    else if (held > 1500) {
                        btn->callback(MEDIUM_PRESS);
                    }
                }
            }
        }
        else if(btn->is_pressed && now - btn->last_seen > 40){
            btn->is_pressed = false;
        }
    }
}

#define BTN_BG NDSU_YELLOW
#define BTN_FG 0


#define HOME_BB_X 420
#define HOME_BB_Y   0
#define HOME_BB_W  60
#define HOME_BB_H  45

#define HOME_TXT_X 430
#define HOME_TXT_Y  30



void ui_display_btns(){
    bounding_box_create_text(PLUS_IDX, 130, 250, "+", FONT_24PT);

    BoundingBox p = boxes[PLUS_IDX];

    uint16_t x,y,w,h;
    uint16_t padding = 12;

    x = p.x - padding;
    y = p.y - p.h - padding;
    w = p.w + 2*padding;
    h = p.h + 2*padding;

    display_draw_box(x, y, w, h, BTN_BG);
    bounding_box_draw_text(PLUS_IDX, "+", BTN_FG, FONT_24PT);
    bounding_box_update(PLUS_IDX, x, y, w, h, 0);

    x = 330;
    y = 237;
    w = 22;
    h = 2;
    bounding_box_update(MINUS_IDX, x - padding, boxes[PLUS_IDX].y, boxes[PLUS_IDX].w, boxes[PLUS_IDX].h, 0);
    bounding_len++;

    display_draw_box(boxes[MINUS_IDX].x, boxes[MINUS_IDX].y, boxes[MINUS_IDX].w, boxes[MINUS_IDX].h, BTN_BG);
    display_draw_box(x, y, w, h, BTN_FG);


    bounding_box_create_text(ON_OFF_IDX, 200,250, "On/Off", FONT_12PT);

    x = boxes[PLUS_IDX].x + boxes[PLUS_IDX].w + 15;
    y = boxes[PLUS_IDX].y;
    w = boxes[MINUS_IDX].x - x - 15;
    h = boxes[PLUS_IDX].h;

    display_draw_box(x, y, w, h, BTN_BG);
    bounding_box_draw_text(ON_OFF_IDX, "On/Off", BTN_FG, FONT_12PT);
    bounding_box_update(ON_OFF_IDX, x, y, w, h, 0);

    bounding_box_update(MORE_IDX, HOME_BB_X, HOME_BB_Y, HOME_BB_W, HOME_BB_H, 0);
    bounding_len++;
    display_draw_box(HOME_BB_X, HOME_BB_Y, HOME_BB_W, HOME_BB_H, BTN_BG);
    display_draw_text("More", HOME_TXT_X, HOME_TXT_Y, BTN_FG, FONT_9PT);

    btn_create(PLUS_IDX, increment_time_cb);
    btn_create(MINUS_IDX, decrement_time_cb);
    btn_create(ON_OFF_IDX, start_stop_cb);
    btn_create(MORE_IDX, more_cb);
}



#define TOGGLE_IDX 0
#define UTC_IDX    1
#define HOME_IDX   2

#define SETT_PADDING 10

#define TOGGLE_BB_X 140
#define TOGGLE_BB_Y  70
#define TOGGLE_BB_W 190
#define TOGGLE_BB_H  45


#define TOGGLE_TXT_X  TOGGLE_BB_X + 40
#define TOGGLE_TXT_Y  TOGGLE_BB_Y + 30

#define UTC_BB_X  TOGGLE_BB_X
#define UTC_BB_Y  TOGGLE_BB_Y + TOGGLE_BB_H + SETT_PADDING
#define UTC_BB_W  TOGGLE_BB_W
#define UTC_BB_H  TOGGLE_BB_H

#define UTC_TXT_X  UTC_BB_X + 50
#define UTC_TXT_Y  UTC_BB_Y + 30


static void toggle_temp_cb(__unused ButtonPress press){
    program_state.is_celsius = !program_state.is_celsius;
    char toggle_buff[9] = "Toggle ";
    toggle_buff[7] = (program_state.is_celsius) ? 'C' : 'F';
    toggle_buff[8] = 0;

    display_draw_box(TOGGLE_BB_X,TOGGLE_BB_Y, TOGGLE_BB_W, TOGGLE_BB_H, BTN_BG);
    display_draw_text(toggle_buff, TOGGLE_TXT_X, TOGGLE_TXT_Y, BTN_FG, FONT_12PT);

}

static void utc_cb(__unused ButtonPress press){
   program_state.utc_offset++;
   if(program_state.utc_offset > 14) program_state.utc_offset = -12;
   char utc_buff[8] = "UTC+00";
   int offset = program_state.utc_offset;
   utc_buff[3] = '+';
   if(offset < 0){
       offset = -offset;
       utc_buff[3] = '-';
   }
   utc_buff[4] = '0' + offset / 10;
   utc_buff[5] = '0' + offset % 10;
   utc_buff[6] = '\0';

   display_draw_box( UTC_BB_X, UTC_BB_Y, UTC_BB_W, UTC_BB_H, BTN_BG);
   display_draw_text(utc_buff, UTC_TXT_X, UTC_TXT_Y, BTN_FG, FONT_12PT);
}

static void home_cb(__unused ButtonPress press){
    ui_clear(NDSU_GREEN);
    program_state.prev_screen = program_state.screen;
    program_state.screen = HOME_SCREEN;
    ui_display_btns();
}


void ui_draw_settings_screen(){
    char toggle_buff[9] = "Toggle ";
    toggle_buff[7] = (program_state.is_celsius) ? 'C' : 'F';
    toggle_buff[8] = 0;

    bounding_box_update(TOGGLE_IDX, TOGGLE_BB_X, TOGGLE_BB_Y, TOGGLE_BB_W, TOGGLE_BB_H, 0);
    bounding_len++;
    display_draw_box(TOGGLE_BB_X,TOGGLE_BB_Y, TOGGLE_BB_W, TOGGLE_BB_H, BTN_BG);
    display_draw_text(toggle_buff, TOGGLE_TXT_X, TOGGLE_TXT_Y, BTN_FG, FONT_12PT);
    btn_create(TOGGLE_IDX, toggle_temp_cb);


    char utc_buff[8] = "UTC+00";
    int offset = program_state.utc_offset;
    utc_buff[3] = '+';
    if(offset < 0){
        offset = -offset;
        utc_buff[3] = '-';
    }
    utc_buff[4] = '0' + offset / 10;
    utc_buff[5] = '0' + offset % 10;
    utc_buff[6] = '\0';

    bounding_box_update(UTC_IDX, UTC_BB_X, UTC_BB_Y, UTC_BB_W, UTC_BB_H, 0);
    bounding_len++;
    display_draw_box( UTC_BB_X, UTC_BB_Y, UTC_BB_W, UTC_BB_H, BTN_BG);
    display_draw_text(utc_buff, UTC_TXT_X, UTC_TXT_Y, BTN_FG, FONT_12PT);
    btn_create(UTC_IDX, utc_cb);


    bounding_box_update(HOME_IDX, HOME_BB_X, HOME_BB_Y, HOME_BB_W, HOME_BB_H, 0);
    bounding_len++;
    display_draw_box(HOME_BB_X, HOME_BB_Y, HOME_BB_W, HOME_BB_H, BTN_BG);
    display_draw_text("Home", HOME_TXT_X, HOME_TXT_Y, BTN_FG, FONT_9PT);
    btn_create(HOME_IDX, home_cb);
}
