// SDA pin
#define SDA_GPIO                4

// SCL pin
#define SCL_GPIO                2

// I2C baud rate
#define BAUD_SPEED              400000

// Cryptochip type
#define ECCX08_TYPE             "608"

// Cryptochip address
#define ECCX08_ADDRESS          0XC0
#include "port.h"
#include "cryptoauthlib.h"

/*
 * Initialize ATCA configuration
 */
ATCAIfaceCfg atca_cfg_init()
{
	ATCADeviceType type;

	if(strcmp(ECCX08_TYPE, "608") == 0) type = ATECC608A;
	else if(strcmp(ECCX08_TYPE, "508") == 0) type = ATECC508A;
	else if(strcmp(ECCX08_TYPE, "204") == 0) type = ATSHA204A;
	else if(strcmp(ECCX08_TYPE, "108") == 0) type = ATECC108A;
	else type = ATCA_DEV_UNKNOWN;

	ATCAIfaceCfg cfg = 
	{
		.iface_type 			= ATCA_I2C_IFACE,
		.devtype				= type,
		.atcai2c.slave_address	= ECCX08_ADDRESS,
		.atcai2c.bus			= I2C_NUM_1,
		.atcai2c.baud			= BAUD_SPEED,
		.wake_delay				= 1500,
		.rx_retries				= 20
	};
	return cfg;
}