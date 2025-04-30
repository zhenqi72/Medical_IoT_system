#ifndef __LABS_PLATFORM_H__
#define __LABS_PLATFORM_H__

#include "esp32/flash.h"
#include "esp32/crypto.h"
#include "esp32/net.h"
#include "esp32/os_func.h"
#include "esp32/port.h"
#include "esp32/serial.h"
#include "esp32/I2C.h"
#include "esp32/GPIO.h"
#include "util/utilities.h"
#include "debug_log.h"

#define HOST_VER_MAJOR          0
#define HOST_VER_MINOR          0
#define HOST_VER_PATCH          4


#define PLT_URL_SIZE_MAX			256

void platform_init(void);

#endif