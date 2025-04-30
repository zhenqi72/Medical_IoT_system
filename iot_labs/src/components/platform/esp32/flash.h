#ifndef _PLT_FLASH_H_
#define _PLT_FLASH_H_

#include "port.h"

#define FLASH_SECTOR_SIZE                   SPI_FLASH_SEC_SIZE
#define ESP_FLASH_SECTOR_INDEX(addr)        (0x1C0000 >>0x1000)
/**
 * @brief read data from flash
 * @param addr Address to read data from
 * @param buff Pointer to buffer to save flash data to
 * @param len Length of buffer
 * @retval Status of read, 0 if successful
 */
int plt_flash_read(uint32_t addr, uint8_t* buf, size_t len);
/**
 * @brief erase data from flash
 * @param addr Address to erase data from
 * @retval Status of read, 0 if successful
 */
int plt_flash_erase(uint32_t addr);
/**
 * @brief write data to flash
 * @param addr Address to erase data from
 * @param buff Pointer to the data to write
 * @param len Length of buffer
 * @retval Status of write, 0 if successful
 */
int plt_flash_write(uint32_t addr, uint8_t* buf, size_t len);

#endif