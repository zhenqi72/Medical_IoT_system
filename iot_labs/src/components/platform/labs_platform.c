#include "labs_platform.h"

void _print_dev_info()
{
    LOG_PRINTF("\r\n");
    LOG_PRINTF("    ██     ██ ███████ ██       ██████  ██████  ███    ███ ███████ ");
    LOG_PRINTF("    ██     ██ ██      ██      ██      ██    ██ ████  ████ ██      ");
    LOG_PRINTF("    ██  █  ██ █████   ██      ██      ██    ██ ██ ████ ██ █████   ");
    LOG_PRINTF("    ██ ███ ██ ██      ██      ██      ██    ██ ██  ██  ██ ██      ");
    LOG_PRINTF("     ███ ███  ███████ ███████  ██████  ██████  ██      ██ ███████ ");
    LOG_PRINTF("\r\n");

    efuse_init();

    uint8_t mac[6] = {0};
    get_mac(mac);
    esp_base_mac_addr_set(mac);

    //printf dev info
    LOG_PRINTF("--------------------------- Device Info ---------------------------");
    LOG_PRINTF("MAC ADDRESS: "MACSTR"", MAC2STR(mac));
    LOG_PRINTF("MCU MODEL: %s", get_mcu_model());
    LOG_PRINTF("VERSION: %d.%d.%d", HOST_VER_MAJOR, HOST_VER_MINOR, HOST_VER_PATCH);
    LOG_PRINTF("BUILD TIME: %s, %s", __DATE__,__TIME__);
    LOG_PRINTF("------------------------ Let's Get Started ------------------------");
    LOG_PRINTF("\r\n");

    LOG_PRINTF("* Enter the password to enable debug console.");
}

void platform_init(void)
{    
    _print_dev_info();
    // Initialize periphrials
    serial_init(UART_NUM_0);
    if(0 != i2c_init(I2C_NUM_0))
    {
        LOG_DEBUG("Failed to Initialize I2C for screen");
    }
    // if(0 != i2c_init(I2C_NUM_1))
    // {
    //     LOG_DEBUG("Failed to Initialize I2C for ATECC608");
    // }
}
