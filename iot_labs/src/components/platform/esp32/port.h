#ifndef _PORT_H_
#define _PORT_H_

#define MCU_MODEL      "ESP32"
// Time
#include <time.h>
#include "sys/time.h"
#include "apps/sntp/sntp.h"
//Drivers
#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/i2c.h"
#include "hwcrypto/aes.h"
#include "rom/efuse.h"
#include "rom/gpio.h"
#include "rom/queue.h"
#include "rom/uart.h"
#include "rom/md5_hash.h"
#include "rom/spi_flash.h"
#include "soc/soc.h"
#include "soc/uart_reg.h"
#include "soc/efuse_reg.h"

//ESP
#include "esp_err.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_partition.h"
#include "esp_ota_ops.h"
#include "sys/types.h"
#include "esp_intr_alloc.h"

//nvs (storage)
#include "nvs_flash.h"
#include "nvs.h"

//FreeRTOS
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/portmacro.h"
#include "freertos/portable.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/event_groups.h"

//Net
#include "lwip/api.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/timers.h"
#include "lwip/sockets.h"

#include "labs_defines.h"

/**
 * @brief Do a efuse read operation, to update the efuse value to efuse read registers. Initializes MAC array.
 */
void efuse_init();
/**
 * @brief Copies the MAC address into the buffer
 * @param[out] buf Buffer to copy the MAC into
 */
void get_mac(uint8_t buf[6]);
/**
 * @brief Gets the type of MCU
 * @return Pointer to the MCU model
 */
char* get_mcu_model(void);
/**
 * @brief Gets the idf version for the ESP
 * @return Pointer to the IDF version
 */
const char* get_idf_version(void);

#endif