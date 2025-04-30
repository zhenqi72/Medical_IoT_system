#ifndef __PLT_OTA_H__
#define __PLT_OTA_H__

#include "flash.h"

typedef struct{
	size_t address;
	size_t size;
}ota_flash_partition_t;

typedef struct ota_flash_info {
	ota_flash_partition_t image_partitions[2];
	ota_flash_partition_t upgrade_partitions[4];
	void *ext;
}ota_flash_info_t;
/**
 * @brief Prepare flash for over the air update
 * @param pf Pointer the the flash to use
 * @param ota_size Size of update
 * @return 0 on success -1 on failure
 */
int ota_flash_prepare(ota_flash_info_t *pf, size_t ota_size);
/**
 * @brief Write data to flash and erase previous data
 * @param pf Pointer to the flash to use
 * @param offset Offset to where the data starts
 * @param pdata Pointer to the data to write
 * @param len Size of the data to write
 * @return 0 on success -1 on failure
 */
int ota_write_flash_with_erase(ota_flash_info_t *pf, int offset, uint8_t *pdata, size_t len);
/**
 * @brief Write data to flash
 * @param pf Pointer to the flash to write to
 * @param offset Offset to begin write to
 * @param pdata Pointer to data to write
 * @param len Size of the data to write
 * @return 0 on success -1 on failure
 */
int ota_write_flash(ota_flash_info_t *pf, int offset, uint8_t *pdata, size_t len);
/**
 * @brief Erase data in flash
 * @param pf Poiner to the flash to erase from
 * @param offset Offset to begin erase from
 * @param len Amount of data to erase
 * @return 0 on success -1 on failure
 */
int ota_erase_flash(ota_flash_info_t *pf, int offset, size_t len);
/**
 * @brief Read data from flash
 * @param pf Pointer to the flash to read from
 * @param offset Offset to begin read from
 * @param pdata Buffer to store data to
 * @param len Amount of data to read (size of buffer)
 * @return 0 on success -1 on failure
 */
int ota_read_flash(ota_flash_info_t *pf, int offset, uint8_t *pdata, size_t len);
/**
 * @brief Configure OTA data for new boot partition
 * @param pf Pointer to flash containing partion
 * @return 0 on success -1 on failure
 */
int ota_set_flash_boot(ota_flash_info_t *pf);
/**
 * @brief Verify OTA data
 */
int ota_verify(ota_flash_info_t *pf, int offset, size_t len);

#endif
