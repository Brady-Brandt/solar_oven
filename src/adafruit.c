/*
 * Implements a lot of the http client code in the pico examples repo
 * Changed some of the api to better fit my preferences
 * This file mainly sets up the https specifically to upload
 * data to the temperature feed for adafruit io
 */

/**
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <pico/time.h>
#ifdef ENABLE_WIFI
#include <stdint.h>

#include "https.h"
#include "debug.h"

#include "lwip/altcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/pbuf.h"

typedef struct {
    /*!
     * The name of the host, e.g. www.raspberrypi.com
     */
    const char *hostname; 
    /*!
     * Function to callback with headers, can be null
     * @see httpc_headers_done_fn
     */
    httpc_headers_done_fn headers_fn;
    /*!
     * Function to callback with results from the server, can be null
     * @see altcp_recv_fn
     */
    altcp_recv_fn recv_fn;
    /*!
     * Function to callback with final results of the request, can be null
     * @see httpc_result_fn
     */
    httpc_result_fn result_fn;
    /*!
     * Callback to pass to calback functions
     */
    void *callback_arg; 
    /*!
     * TLS allocator, used internall for setting TLS server name indication
     */
    altcp_allocator_t tls_allocator;
    /*!
     * LwIP HTTP client settings
     */
    httpc_connection_t settings;
    /*!
     * Flag to indicate when the request is complete
     */
    int complete;
    /*!
     * Flag to indicate if it is the first request
     */
    int not_first_request;
    /*!
     * Overall result of http request, only valid when complete is set
     */
    httpc_result_t result;

} HTTPSRequest;


#define HTTPS_PORT 443
#define ADA_PTEMP_URL "/api/v2/"  ADAFRUIT_USERNAME  "/feeds/temperature/data"

static struct altcp_tls_config* tls_config = 0;
static HTTPSRequest adafruit_req = {0};



// Print headers to stdout
err_t http_client_header_print_fn(__unused httpc_state_t *connection, __unused void *arg, 
        struct pbuf *hdr, uint16_t hdr_len, __unused uint32_t content_len) {
    debug_info("\nheaders %u\n", hdr_len);
    uint16_t offset = 0;
    while (offset < hdr->tot_len && offset < hdr_len) {
        char c = (char)pbuf_get_at(hdr, offset++);
        debug_print("%c",c);
    }
    return ERR_OK;
}

// Print body to stdout
err_t http_client_receive_print_fn(__unused void *arg, __unused struct altcp_pcb *conn, struct pbuf *p, err_t err) {
    debug_info("\ncontent err %d\n", err);
    uint16_t offset = 0;
    while (offset < p->tot_len) {
        char c = (char)pbuf_get_at(p, offset++);
        debug_print("%c",c);
    }
    pbuf_free(p);
    return ERR_OK;
}


static err_t internal_header_fn(httpc_state_t *connection, void *arg, struct pbuf *hdr, uint16_t hdr_len, uint32_t content_len) {
    assert(arg);
    HTTPSRequest *req = (HTTPSRequest*)arg;
    if (req->headers_fn) {
        return req->headers_fn(connection, req->callback_arg, hdr, hdr_len, content_len);
    }
    return ERR_OK;
}

static err_t internal_recv_fn(void *arg, struct altcp_pcb *conn, struct pbuf *p, err_t err) {
    assert(arg);
    HTTPSRequest *req = (HTTPSRequest*)arg;
    if (req->recv_fn) {
        return req->recv_fn(req->callback_arg, conn, p, err);
    }
    return ERR_OK;
}

static void internal_result_fn(void *arg, httpc_result_t httpc_result, uint32_t rx_content_len, uint32_t srv_res, err_t err) {
    assert(arg);
    HTTPSRequest *req = (HTTPSRequest*)arg;
    debug_info("result %d len %u server_response %u err %d\n", 
            httpc_result, rx_content_len, srv_res, err);
    req->complete = true;
    req->result = httpc_result;
    if (req->result_fn) {
        req->result_fn(req->callback_arg, httpc_result, rx_content_len, srv_res, err);
    }
}

// Override altcp_tls_alloc to set sni
static struct altcp_pcb *altcp_tls_alloc_sni(void *arg, u8_t ip_type) {
    assert(arg);
    HTTPSRequest *req = (HTTPSRequest*)arg;
    struct altcp_pcb *pcb = altcp_tls_alloc(tls_config, ip_type);
    if (!pcb) {
        debug_err("Failed to allocate PCB\n");
        return NULL;
    }
    mbedtls_ssl_set_hostname(altcp_tls_context(pcb), req->hostname);
    return pcb;
}

// Make a http request, complete when req->complete returns true
static int http_client_request_async(HTTPSRequest* req, const char* url) {
    if(!tls_config){
        tls_config = altcp_tls_create_config_client(NULL, 0);
        req->tls_allocator.alloc = altcp_tls_alloc_sni;
        req->tls_allocator.arg = req; 
        req->settings.altcp_allocator = &req->tls_allocator;
    }


    req->complete = false;
    req->settings.headers_done_fn = internal_header_fn;
    req->settings.result_fn = internal_result_fn;
    err_t ret = httpc_get_file_dns(req->hostname, HTTPS_PORT, 
            url, &req->settings, internal_recv_fn, req, NULL);
    if (ret != ERR_OK) {
        debug_err("http request failed: %d", ret);
    }
    return ret;
}

bool adafruit_send_temperatue(__unused repeating_timer_t* rt){
    if(!adafruit_req.not_first_request){
        adafruit_req.headers_fn = http_client_header_print_fn;
        adafruit_req.recv_fn    = http_client_receive_print_fn;
        adafruit_req.hostname   = "io.adafruit.com";
        http_client_request_async(&adafruit_req, ADA_PTEMP_URL);
    } else if (adafruit_req.not_first_request && adafruit_req.complete) { 
        //only want to call another request if the old one finishes
        http_client_request_async(&adafruit_req, ADA_PTEMP_URL);
    }
    return true;
}
#else
bool adafruit_send_temperatue(__unused repeating_timer_t* rt) {return true;}
#endif // ENABLE_WIFI
