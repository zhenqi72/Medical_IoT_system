#include "labs.h"
#include "host.h"
#include "os_func.h"
#include "cryptoauthlib.h"
#include "atcacert/atcacert_client.h"
#include "atcacert/atcacert_host_hw.h"
#include "driver/i2c.h"
#include "crypto_settings.h"
#include "freertos/task.h"
#include "csr_def.h"
#include "led.h"
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <stdio.h>

socket_tcp_t tcp;
int *tcp_p=&tcp;
socket_tcp_config_t configs= TCP_CONFIG_INIT_VALUE ;
int *configs_p = &configs;
const char *payload = "Message from ESP32 ";
char rx_buffer[128];

/**
 * Main 
 */

void app_main(void)
{
    host_config_t host_config = {
        .encrypt_type = ENCRYPT_TYPE_AES,
        .console_uart_num = UART_NUM_0,
        .wifi_if = {
            .wifi_init = labs_wifi_init,
            .wifi_disconnect = NULL
        },
        .plt_if = {
            .drivers_init = platform_init,
            .time_init = utc_set
        }
    };

    host_handle_t host_handle = host_create(&host_config);

    ms_sleep(1000);
	
    led_init();
	socket_tcp_open(tcp_p, configs_p);
    socket_tcp_write_to(tcp_p, payload, sizeof(payload));
    uint8_t count = 0;
    while(1)
    {   
        socket_tcp_read_from(tcp_p, rx_buffer, sizeof(rx_buffer));
		led_display_num(count++);
        vTaskDelay(MS2TICK(500));
    }
}
