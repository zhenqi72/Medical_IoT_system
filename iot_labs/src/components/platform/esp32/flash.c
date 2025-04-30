#include "flash.h"

#define FLASH_ADDR_2_SECTOR_INDEX(addr)      (addr>>12)

int plt_flash_erase(uint32_t addr)
{
    int ret = spi_flash_erase_sector(FLASH_ADDR_2_SECTOR_INDEX(addr));

    return (ESP_OK == ret) ? 0 : -1;
}

int plt_flash_write(uint32_t addr, uint8_t* buf, size_t len)
{
	uint32_t temp;
	uint8_t *tempbuf = (uint8_t *)&temp;
	uint32_t addr_base = addr;
	uint8_t *p = buf;
	uint8_t *newbuf = NULL;
	size_t write_len = len;
	uint32_t offset = 0;
	size_t last = 0;

	if(0 == write_len){
		return 0;
	}

    //addr is not aligned
	if (addr & 0x03) {
		addr_base = addr & 0xFFFFFFFC;
		offset = addr & 0x03;
		last = 4 - offset;
		if (write_len < last) {
			last = write_len;
		}
		write_len -= last;
		spi_flash_read(addr_base, tempbuf, 4);
		while (last > 0) {
			tempbuf[offset++] = *p++;
			last--;
		}
		spi_flash_write(addr_base, tempbuf, 4);
		addr_base += 4;
	}

	if(0 == write_len){
		return 0;
	}

    /* Write blocks */
	last = write_len - (write_len & 0x03);
	if (((uint32_t)p) & 0x03) {
		while(last > 0){
			tempbuf[0] = *p++;
			tempbuf[1] = *p++;
			tempbuf[2] = *p++;
			tempbuf[3] = *p++;
			spi_flash_write(addr_base, tempbuf, 4);
			last -= 4;
			addr_base += 4;
		}
	}
	else if(last > 0){
		spi_flash_write(addr_base, p, last);
		p += last;
		addr_base += last;
	}

    /* Write last */
	last = write_len & 0x03;
	if (last > 0) {
		memset(tempbuf, 0xff, 4);
		offset = 0;
		while (last > 0) {
			tempbuf[offset++] = *p++;
			last--;
		}
		spi_flash_write(addr_base, tempbuf, 4);
		addr_base += 4;
    }

	if (newbuf) {
		free((void*)newbuf);
		newbuf = NULL;
	}
    
	return 0;
}

int plt_flash_read(uint32_t addr, uint8_t* buf, size_t len)
{
	uint32_t temp;
	uint8_t *tempbuf = (uint8_t *)&temp;
	uint32_t addr_base = addr;
	uint8_t* p = buf;
	size_t readLen = len;
	uint32_t offset = 0;
	size_t last = 0;

	if(0 == readLen){
		return 0;
	}

    //addr is not aligned
	if (addr & 0x03) {
		addr_base = addr & 0xFFFFFFFC;
		offset = addr & 0x03;
		last = 4 - offset;
		if (readLen < last) {
			last = readLen;
		}
		readLen -= last;
		spi_flash_read(addr_base, tempbuf, 4);
		while (last > 0) {
			*p++ = tempbuf[offset++];
			last--;
		}
		addr_base += 4;
	}

	if(0 == readLen){
		return 0;
	}

	/* Read blocks */
	last = readLen - (readLen & 0x03);
	if (((uint32_t)p) & 0x03) {
		while(last > 0){
			spi_flash_read(addr_base, tempbuf, 4);
			*p++ = tempbuf[0];
			*p++ = tempbuf[1];
			*p++ = tempbuf[2];
			*p++ = tempbuf[3];
			last -= 4;
			addr_base += 4;
		}
	}
	else if(last > 0){
		spi_flash_read(addr_base, p, last);
		p += last;
		addr_base += last;
	}

	/* Read last */
	last = readLen & 0x03;
	if(last > 0){
		offset = 0;
		spi_flash_read(addr_base, tempbuf, 4);
		while (last > 0) {
			*p++ = tempbuf[offset++];
			last--;
		}
		addr_base += 4;
	}

	return 0;
}
