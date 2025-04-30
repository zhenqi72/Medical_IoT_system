#include "fw_ota.h"
#include "labs_platform.h"

// host I/O interfaces

static int _fw_ota_open(void *stream_ctx, void* len)
{
    fw_ota_t *ota_ctx = (fw_ota_t *)stream_ctx;
    ota_flash_info_t *flash = &ota_ctx->flash;

    ota_ctx->offset = 0;
    ota_ctx->content_len = *(uint32_t*)len;

    //progress init

    return ota_flash_prepare(flash, ota_ctx->content_len);
}

static int _fw_ota_write(void *stream_ctx, uint8_t *buf, size_t len)
{
    fw_ota_t *ota_ctx = (fw_ota_t *)stream_ctx;
    ota_flash_info_t *flash = &ota_ctx->flash;

    int ret = ota_write_flash_with_erase(flash, ota_ctx->offset, buf, len);
    ota_ctx->offset += len;

    if(0 != ret) {
		LOG_ERROR("fw write error, return %d", ret);
		return -1;
	}
    //progress update

    return len;
}

static int _fw_ota_close(void *stream_ctx)
{
    fw_ota_t *ota_ctx = (fw_ota_t *)stream_ctx;
    ota_flash_info_t *flash = &ota_ctx->flash;

    ota_set_flash_boot(flash);
    ms_sleep(3000);
    reboot();

    return 0;
}

static int _fw_ota_error(void *stream_ctx, void *msg)
{
    LOG_ERROR("client_http: Error");
    //retry?

    return 0;
}

//read

int fw_ota_init(fw_ota_t *ota_ctx)
{
    if(NULL != ota_ctx) {
        memset(ota_ctx, 0, sizeof(fw_ota_t));

        fw_ota_if_t ota_if_config = {
            .fp_open = _fw_ota_open,
            .fp_read = NULL,
            .fp_write = _fw_ota_write,
            .fp_close = _fw_ota_close,
            .fp_error = _fw_ota_error
        };
        memcpy(&ota_ctx->ota_if, &ota_if_config, sizeof(fw_ota_if_t));
        return 0;
    }
    LOG_ERROR("fw_ota_init: ota_ctx is null!");
    return -1;
}

int fw_ota_deinit(fw_ota_t *ota_ctx)
{  
    if (ota_ctx) {
        memset(ota_ctx, 0, sizeof(fw_ota_t));
        free(ota_ctx);
    }

    return 0;
}
