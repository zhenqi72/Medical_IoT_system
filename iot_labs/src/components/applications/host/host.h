#ifndef __HOST_H__
#define __HOST_H__

#include "labs_wifi_ctrl.h"
#include "debug_log.h"

#include "client_udp.h"
#include "fw_ota.h"
#include "console.h"
#include "htu21d.h"
#include "oled.h"

//wifi interfaces
typedef struct {
    void (*wifi_init)(void);
    void (*wifi_disconnect)(void);
    //add more 
}host_wifi_if_t;

//platform interfaces
typedef struct {
    void (*drivers_init)(void);
    void (*time_init)(void);
    // ADD mutex for the I2C bus
}host_plt_if_t;

//host interfaces
typedef struct {
    host_wifi_if_t wifi_if;
    host_plt_if_t plt_if;
}host_if_t;

//host
typedef struct _host{
    uint8_t mac[6];
    client_udp_t* client_udp;//communication clients
    console_t* console;//uart console & cmd handler
    htu21_t* htu21;
    ssd1306_t* ssd1306;
    //ota downloader
    
    host_if_t host_if;//interfaces
}host_t;

typedef struct {
    pkt_encrypt_t encrypt_type;
    int console_uart_num;
    host_wifi_if_t wifi_if;
    host_plt_if_t plt_if;
}host_config_t;

typedef void* host_handle_t;

host_handle_t host_create(host_config_t *host_config);
void host_destory(host_handle_t host_handler);

#endif
