#pragma once

/**
 * @brief Initializes the Wi-Fi hardware on the Pico.
 *
 * @return PICO_ERROR_NONE on success
 *
 */
enum pico_error_codes wifi_init(); 


/**
 * Connects to a Wi-Fi network with the given SSID and password.
 *
 * @param ssid the SSID (network name) to connect to
 * @param pwd  the password for the Wi-Fi network
 * @return PICO_ERROR_NONE on success
 *
 */
enum pico_error_codes wifi_connect(char* ssid, char* pwd);

/**
 * @brief Synchronizes the Raspberry Pi Pico's RTC (real-time clock) with NTP 
 */
void sync_rtc();
