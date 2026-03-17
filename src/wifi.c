#include <pico/error.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define CONNECTION_TIMEOUT 5000

#ifdef ENABLE_WIFI
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
enum pico_error_codes wifi_init(){
    enum pico_error_codes err = cyw43_arch_init_with_country(CYW43_COUNTRY_USA);
    if(err != PICO_ERROR_NONE) return err;

    cyw43_arch_enable_sta_mode();
    
    return PICO_ERROR_NONE;
}

enum pico_error_codes wifi_connect(char* ssid, char* pwd){
    return cyw43_arch_wifi_connect_timeout_ms(ssid, pwd, CYW43_AUTH_WPA2_AES_PSK, CONNECTION_TIMEOUT);

}

void wifi_get_mac_address(char addr[19]){
    uint8_t mac[8];
    cyw43_wifi_get_mac(&cyw43_state,  CYW43_ITF_STA,mac); 
    snprintf(addr, 19,"%02X:%02X:%02X:%02X:%02X:%02X\n",mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

#else
enum pico_error_codes wifi_init(){ 
    return PICO_ERROR_NOT_FOUND;
}

enum pico_error_codes wifi_connect(char* ssid, char* pwd){
    return PICO_ERROR_NOT_FOUND;

}

void wifi_get_mac_address(char addr[19]){
    snprintf(addr, 19,"%02X:%02X:%02X:%02X:%02X:%02X\n",0, 0,0, 0, 0, 0);
}


#endif //ENABLE_WIFI
