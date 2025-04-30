#include "host.h"

host_handle_t host_create(host_config_t *host_config)
{
    if(NULL == host_config || NULL == host_config->wifi_if.wifi_init || NULL == host_config->plt_if.drivers_init){
		LOG_ERROR("Invalid host configuration!");
		return NULL;
	}

    //host
    host_t *host = malloc(sizeof(host_t));
    if(NULL != host) {
        memset(host, 0, sizeof(host_t));
    }
    else
    {
        LOG_ERROR("host malloc failed.");
        return NULL;
    }

    //mac
    get_mac(host->mac);


    //host interfaces
    host->host_if.plt_if = host_config->plt_if;
    host->host_if.wifi_if = host_config->wifi_if;

    //init
    host->host_if.plt_if.drivers_init();
    host->host_if.wifi_if.wifi_init();

    //client init
    host->client_udp = malloc(sizeof(client_udp_t));
    if(NULL != host->client_udp) {
        memset(host->client_udp, 0, sizeof(client_udp_t));
    }
    else
    {
        LOG_ERROR("Client UDP malloc failed.");
        goto err_exit;
    }
    
    client_udp_init(host->client_udp, host_config->encrypt_type);

    //console init
    host->console = malloc(sizeof(console_t));
    if(NULL != host->console) {
        memset(host->console, 0, sizeof(console_t));
    }
    else
    {
        LOG_ERROR("Console malloc failed.");
        goto err_exit;
    }
    
    console_init(host->console, host_config->console_uart_num);

    client_udp_set_mode(host->client_udp, CLIENT_UDP_MODE_ONLINE);
    
    // Initialize the time
    host->host_if.plt_if.time_init();
    // TODO: Might be a good idea to create init functions for the peripherials
    // Initialize the modules
    // htu21d temperature sensor
    host->htu21 = malloc(sizeof(htu21_t));
    if(NULL != host->htu21)
    {
        memset(host->htu21, 0, sizeof(htu21_t));
    }
    else
    {
        LOG_ERROR("HTU21D malloc failed.");
        goto err_exit;
    }
    if(0 != queue_create(&(host->htu21->msg_queue), 1, sizeof(htu21_data_t)))
    {
        LOG_ERROR("HTU21D queue create failed!");
        goto err_exit;
    }
    thread_create(&(host->htu21->thread), "HTU_thread", htu21_main, 2048, (void*)host->htu21, PRIORITY_DEFAULT);
    // OLED
    host->ssd1306 = malloc(sizeof(ssd1306_t));
    if(NULL != host->ssd1306)
    {
        memset(host->ssd1306, 0, sizeof(ssd1306_t));
    }
    else
    {
        LOG_ERROR("SSD1306 malloc failed.");
        goto err_exit;
    }
    host->ssd1306->state = ENV_INFO;
    thread_create(&(host->ssd1306->thread), "OLED_thread", ssd1306_main, 2048, (void*)host, PRIORITY_DEFAULT);

    return (host_handle_t*)host;

err_exit:

    if(host->client_udp) {
        free(host->client_udp);
        host->client_udp = NULL;
    }

    if(host->mac)
        memset(host->mac, 0, 6);

    if(host) {
        free(host);
        host = NULL;
    }

    return NULL;
}

void host_destory(host_handle_t host_handle)
{
    if(host_handle) {
        host_t *host = (host_t *)host_handle;

        //deinit
        client_udp_deinit(host->client_udp);
        console_deinit(host->console);
        host->host_if.wifi_if.wifi_disconnect();
        //host->host_if.plt_if.driver_deinit();

        //free
        if(host->mac)
            memset(host->mac, 0, 6);

        if(host->client_udp) {
            free(host->client_udp);
            host->client_udp = NULL;
        }

        if(host->console) {
            free(host->console);
            host->console = NULL;
        }

        if(host) {
            free(host);
            host = NULL;
        }
    }
}
