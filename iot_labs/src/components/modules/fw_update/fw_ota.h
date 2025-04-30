#ifndef __FW_OTA_H__
#define __FW_OTA_H__

#include "labs_platform.h"
#include "ota.h"

typedef enum {
    FW_OTA_IDLE = 0,
    FW_OTA_DOWNLOADING,
    FW_OTA_FINISHED,
    FW_OTA_INSTALLING,
    FW_OTA_FAILED
}fw_ota_state_t;


typedef struct{
    int (*fp_open)(void* ctx, void *len);
	int (*fp_read)(void* ctx, uint8_t* out, size_t len);
	int (*fp_write)(void* ctx, uint8_t* in, size_t len);
	int (*fp_close)(void* ctx);
	int (*fp_error)(void* ctx, void* msg);
}fw_ota_if_t;

typedef struct{
	uint32_t content_len;
	uint32_t offset;
    fw_ota_if_t ota_if;
	fw_ota_state_t state;
    ota_flash_info_t flash;
	char url[PLT_URL_SIZE_MAX];
}fw_ota_t;

/**
 * @brief Initialize for a firmware update
 * @param ota_ctx Pointer to where the update is stored
 * @return 0 on success -1 on failure
 */
int fw_ota_init(fw_ota_t *ota_ctx);
/**
 * @brief deinitialize for a firmware update
 * @param ota_ctx Pointer to where the update is stored
 * @return 0 on success -1 on failure
 */
int fw_ota_deinit(fw_ota_t *ota_ctx);

#endif
