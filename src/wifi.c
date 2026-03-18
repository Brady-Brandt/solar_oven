#include "wifi.h"
#include <pico/error.h>
#include <stdbool.h>
#include <stdint.h>

#define CONNECTION_TIMEOUT 5000

#ifdef ENABLE_WIFI
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "debug.h"

enum pico_error_codes wifi_init(){
    enum pico_error_codes err = cyw43_arch_init_with_country(CYW43_COUNTRY_USA);
    if(err != PICO_ERROR_NONE){
        debug_err("Failed to Init Wifi\n");
        return err;
    }

    cyw43_arch_enable_sta_mode();
    
    return PICO_ERROR_NONE;
}

enum pico_error_codes wifi_connect(char* ssid, char* pwd){
    return cyw43_arch_wifi_connect_timeout_ms(ssid, pwd, CYW43_AUTH_WPA2_AES_PSK, CONNECTION_TIMEOUT);

}

WifiStatus wifi_status(){
    int cyw43_status = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
    switch (cyw43_status) {
        case CYW43_LINK_DOWN:
            return WIFI_DOWN;

        case CYW43_LINK_JOIN:
            return WIFI_CONNECTED;

        case CYW43_LINK_FAIL:
            return WIFI_FAILED;

        case CYW43_LINK_NONET:
            return WIFI_NONET;

        case CYW43_LINK_BADAUTH:
            return WIFI_BADAUTH;

        default:
            return WIFI_FAILED; // fallback
    }
}

#else
enum pico_error_codes wifi_init(){ 
    return PICO_ERROR_NOT_FOUND;
}

enum pico_error_codes wifi_connect(char* ssid, char* pwd){
    return PICO_ERROR_NOT_FOUND;

}


WifiStatus wifi_status(){
    return WIFI_DOWN;
}

#endif //ENABLE_WIFI
