#include "port.h"

//TODO: packet
static uint8_t mac[6];

void efuse_init()
{
    ets_efuse_read_op();

    //TODO: Other necessary information in eFuse, e.g. version

    uint32_t mac_low = REG_READ(EFUSE_BLK0_RDATA1_REG);
    uint32_t mac_high = REG_READ(EFUSE_BLK0_RDATA2_REG);

    mac[0] = mac_high >> 8 & 0xff;
	mac[1] = mac_high >> 0 & 0xff;
	mac[2] = mac_low >> 24 & 0xff;
	mac[3] = mac_low >> 16 & 0xff;
	mac[4] = mac_low >> 8  & 0xff;
	mac[5] = mac_low >> 0  & 0xff;
}

void get_mac(uint8_t buf[6])
{
	memcpy(buf, mac, 6);
}

char *get_mcu_model(void)
{
	return MCU_MODEL;
}

const char* get_idf_version(void)
{
	return esp_get_idf_version();
}