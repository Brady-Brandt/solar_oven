 /*
Copyright 2020 (c) 2020 Raspberry Pi (Trading) Ltd.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following
   disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "hardware/rtc.h"
#include "state.h"

#ifdef ENABLE_WIFI

#include "pico/cyw43_arch.h"
#include "lwip/dns.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

typedef struct {
    ip_addr_t ntp_server_address;
    struct udp_pcb *ntp_pcb;
    async_at_time_worker_t request_worker;
} NTP_T;

static NTP_T state = {0};


#define NTP_SERVER "pool.ntp.org"
#define NTP_MSG_LEN 48
#define NTP_PORT 123
#define NTP_DELTA 2208988800 // seconds between 1 Jan 1900 and 1 Jan 1970

static void ntp_result(int status, time_t *result) {
    //on sucessful ntp request update the rtc with the result
    if (status == 0 && result) { 
        struct tm *tm_info = gmtime(result);
        datetime_t t = {
            .year  = tm_info->tm_year + 1900,
            .month = tm_info->tm_mon + 1,
            .day   = tm_info->tm_mday,
            .dotw  = tm_info->tm_wday,
            .hour  = tm_info->tm_hour,
            .min   = tm_info->tm_min,
            .sec   = tm_info->tm_sec
        };
        program_state.time_synced = 1;
        rtc_set_datetime(&t); 
    } else{
        // on failure set ntp to epoch time
        datetime_t t = {
            .year  = 1970,
            .month = 1,
            .day   = 1,
            .dotw  = 4,
            .hour  = 0,
            .min   = 0,
            .sec   = 0
        };
        program_state.time_synced = 0;
        rtc_set_datetime(&t);
    }
}

// Make an NTP request
static void ntp_request() { 
    cyw43_arch_lwip_begin();
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, NTP_MSG_LEN, PBUF_RAM);
    uint8_t *req = (uint8_t *) p->payload;
    memset(req, 0, NTP_MSG_LEN);
    req[0] = 0x1b;
    udp_sendto(state.ntp_pcb, p, &state.ntp_server_address, NTP_PORT);
    pbuf_free(p);
    cyw43_arch_lwip_end();
}

// Call back with a DNS result
static void ntp_dns_found(__unused const char *hostname, const ip_addr_t *ipaddr, __unused void *arg) {
    if (ipaddr) {
        state.ntp_server_address = *ipaddr;
        ntp_request();
    } else {
        ntp_result(-1, NULL);
    }
}

// NTP data received
static void ntp_recv(__unused void *arg, __unused struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    uint8_t mode = pbuf_get_at(p, 0) & 0x7;
    uint8_t stratum = pbuf_get_at(p, 1);

    // Check the result
    if (ip_addr_cmp(addr, &state.ntp_server_address) && port == NTP_PORT && p->tot_len == NTP_MSG_LEN &&
            mode == 0x4 && stratum != 0) {
        uint8_t seconds_buf[4] = {0};
        pbuf_copy_partial(p, seconds_buf, sizeof(seconds_buf), 40);
        uint32_t seconds_since_1900 = seconds_buf[0] << 24 | seconds_buf[1] << 16 | seconds_buf[2] << 8 | seconds_buf[3];
        uint32_t seconds_since_1970 = seconds_since_1900 - NTP_DELTA;
        time_t epoch = seconds_since_1970;
        ntp_result(0, &epoch);
    } else {
        ntp_result(-1, NULL);
    }
    pbuf_free(p);
}

// Called to make a NTP request
static void request_worker_fn(__unused async_context_t *context, __unused async_at_time_worker_t *worker){            int err = dns_gethostbyname(NTP_SERVER, &state.ntp_server_address,
        ntp_dns_found, NULL);

    if (err == ERR_OK) {
        ntp_request(); // Cached DNS result, make NTP request
    } else if (err != ERR_INPROGRESS) { // ERR_INPROGRESS means expect a callback
        ntp_result(-1, NULL);
    }
}

// Perform initialisation
static bool ntp_init() { 
    state.ntp_pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
    if (state.ntp_pcb == NULL) {
        return false;
    }
    udp_recv(state.ntp_pcb, ntp_recv, &state);
    state.request_worker.do_work = request_worker_fn;
    state.request_worker.user_data = &state; 
    return true;
}

void sync_rtc() {
    program_state.time_synced = TIME_SYNC_IN_PROGESS;
    if(!ntp_init()) return;
    async_context_add_at_time_worker_in_ms(cyw43_arch_async_context(),  &state.request_worker, 0);
}
#else
void sync_rtc() {
    datetime_t t = {
        .year  = 1970,
        .month = 1,
        .day   = 1,
        .dotw  = 4,
        .hour  = 0,
        .min   = 0,
        .sec   = 0
    };
    program_state.time_synced = 0;
    rtc_set_datetime(&t);
}
#endif //ENABLE_WIFI
