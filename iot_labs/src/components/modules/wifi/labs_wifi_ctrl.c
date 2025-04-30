#include "labs_wifi_ctrl.h"
#include "debug_log.h"

/******************* Wi-Fi Test *******************/
#define TEST_SSID    "iot_project"    // Wi-Fi SSID
#define TEST_PWD     "123456789"     // Wi-Fi Password

static EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;

/********************************************/

static esp_err_t wifi_event_handler_cb(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;

    case SYSTEM_EVENT_STA_GOT_IP:
        LOG_DEBUG("ip="IPSTR", mask="IPSTR", gw="IPSTR"",
            IP2STR(&event->event_info.got_ip.ip_info.ip),
            IP2STR(&event->event_info.got_ip.ip_info.netmask),
			IP2STR(&event->event_info.got_ip.ip_info.gw));
        
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;

    case SYSTEM_EVENT_AP_STACONNECTED:
        LOG_INFO("station "MACSTR" join, AID = %d",
				  MAC2STR(event->event_info.sta_connected.mac),
				  event->event_info.sta_connected.aid);
        break;

    case SYSTEM_EVENT_AP_STADISCONNECTED:
        LOG_INFO("station "MACSTR" leave, AID = %d",
				  MAC2STR(event->event_info.sta_disconnected.mac),
				  event->event_info.sta_disconnected.aid);
        break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;

    default:
        break;
    }
    return ESP_OK;
}

static void wifi_init_sta(void)
{
    wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler_cb, NULL) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = TEST_SSID,
            .password = TEST_PWD
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    LOG_INFO("wifi_init_sta finished.");
    LOG_INFO("connect to ap SSID:%s", TEST_SSID);
}

void labs_wifi_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    LOG_INFO("ESP_WIFI_MODE_STA");
    wifi_init_sta();
}
