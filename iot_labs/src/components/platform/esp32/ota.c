#include "ota.h"
#include "debug_log.h"

int ota_flash_prepare(ota_flash_info_t *flash, size_t ota_size)
{
	const esp_partition_t *partition_0 = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, NULL);
	const esp_partition_t *partition_1 = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, NULL);

    if (partition_0) {
    	flash->image_partitions[0].address = partition_0->address;
	}
	else
	{
		LOG_ERROR("Partition 0 not found");
		return -1;
	}
	
    if(partition_1) {
    	flash->image_partitions[1].address = partition_1->address;
	}
	else
	{
		LOG_ERROR("Partition 1 not found");
		return -1;
	}

	const esp_partition_t *update_partition = NULL;
	int index = 0;
	size_t upgrede_partitions_size = 0;
	memset(flash->upgrade_partitions, 0, sizeof(flash->upgrade_partitions));

	update_partition = esp_ota_get_next_update_partition(NULL);
	if(update_partition) {
		flash->upgrade_partitions[index].address = update_partition->address;;
		flash->upgrade_partitions[index].size = update_partition->size;
		upgrede_partitions_size += update_partition->size;
		index++;
	}
	else
	{
		LOG_ERROR("No valid update partition found.");
		return -1;
	}

	if(index < NELEMENTS(flash->upgrade_partitions)) {
		flash->upgrade_partitions[index].address = 0;
		flash->upgrade_partitions[index].size = 0;
	}

	flash->ext = (void*)update_partition;

	if(ota_size > upgrede_partitions_size) {
		LOG_ERROR("OTA size too large: Size: %d Max: %d", ota_size, upgrede_partitions_size);
		return -1;
	}

	return 0;
}

static ota_flash_partition_t* _find_upgrade_partition(ota_flash_info_t *flash, size_t offset, size_t *partition_offset)
{
	ota_flash_partition_t *upgrade_partition = NULL;
	size_t upgrade_partition_offset = 0;
	size_t upgrade_partitions_size = 0;

	for(int i=0; i < NELEMENTS(flash->upgrade_partitions); i++) {
		if(0 == flash->upgrade_partitions[i].size){
			break;
		}
		if(offset < (upgrade_partitions_size+flash->upgrade_partitions[i].size)) {
			upgrade_partition_offset = offset - upgrade_partitions_size;
			upgrade_partition = &(flash->upgrade_partitions[i]);
			break;
		}
		upgrade_partitions_size += flash->upgrade_partitions[i].size;
	}

	if(partition_offset) {
		*partition_offset = upgrade_partition_offset;
	}

	return upgrade_partition;
}

int ota_write_flash_with_erase(ota_flash_info_t *flash, int offset, uint8_t *pdata, size_t len)
{
	while(len > 0){
		size_t upgrade_partition_offset = 0;
		ota_flash_partition_t *upgrade_partition = _find_upgrade_partition(flash, offset, &upgrade_partition_offset);
		if(NULL == upgrade_partition) {
			return -1;
		}

		size_t w_len = MIN(len, upgrade_partition->size - upgrade_partition_offset);
		size_t w_addr = upgrade_partition->address + upgrade_partition_offset;

        static uint32_t last_erased_sector_addr = 0xFFFFFFFF;

        size_t erase_sector_addr = w_addr & ~(FLASH_SECTOR_SIZE-1);
        size_t e_addr = w_addr + w_len;

        while(erase_sector_addr < e_addr) {
            if ( erase_sector_addr != last_erased_sector_addr ){
                if(0 != plt_flash_erase(erase_sector_addr)){
                    return -1;
                }
                last_erased_sector_addr = erase_sector_addr;
            }
            erase_sector_addr += FLASH_SECTOR_SIZE;
        }

		if(0 != plt_flash_write(w_addr, pdata, w_len)){
			return -1;
		}

		pdata  += w_len;
		len    -= w_len;
		offset += w_len;
	}

	return 0;
}

int ota_write_flash(ota_flash_info_t *flash, int offset, uint8_t *pdata, size_t len)
{
	while(len > 0) {
		size_t upgrade_partition_offset = 0;
		ota_flash_partition_t *upgrade_partition = _find_upgrade_partition(flash, offset, &upgrade_partition_offset);
		if(NULL == upgrade_partition) {
			return -1;
		}

		size_t w_len = MIN(len, upgrade_partition->size - upgrade_partition_offset);
		size_t w_addr = upgrade_partition->address + upgrade_partition_offset;

		if(0 != plt_flash_write(w_addr, pdata, w_len)){
			return -1;
		}

		pdata += w_len;
		len -= w_len;
		offset += w_len;
	}

	return 0;
}

int ota_erase_flash(ota_flash_info_t *flash, int offset, size_t len)
{
	while(len > 0){
		size_t upgrade_partition_offset = 0;
		ota_flash_partition_t *upgrade_partition = _find_upgrade_partition(flash, offset, &upgrade_partition_offset);
		if(NULL == upgrade_partition){
			return -1;
		}

		size_t w_len = MIN(len, upgrade_partition->size - upgrade_partition_offset);
		size_t w_addr = upgrade_partition->address + upgrade_partition_offset;

		size_t erase_sector_addr = w_addr & ~(FLASH_SECTOR_SIZE-1);
		size_t e_addr = w_addr + w_len;

		while(erase_sector_addr < e_addr){
			if(0 != plt_flash_erase(erase_sector_addr)){
				return -1;
			}
			erase_sector_addr += FLASH_SECTOR_SIZE;
		}

		len -= w_len;
	}

	return 0;
}

int ota_read_flash(ota_flash_info_t *flash, int offset, uint8_t *pdata, size_t len)
{
	while(len > 0) {
		size_t upgrade_partition_offset = 0;
		ota_flash_partition_t *upgrade_partition = _find_upgrade_partition(flash, offset, &upgrade_partition_offset);
		if(NULL == upgrade_partition) {
			return -1;
		}

		size_t r_len = MIN(len, upgrade_partition->size - upgrade_partition_offset);
		size_t r_addr = upgrade_partition->address + upgrade_partition_offset;

		if(0 != plt_flash_read(r_addr, pdata, r_len)){
			return -1;
		}

		pdata  += r_len;
		len    -= r_len;
		offset += r_len;
	}

	return 0;
}

int ota_set_flash_boot(ota_flash_info_t *flash)
{
	const esp_partition_t *update_partition = (const esp_partition_t *)flash->ext;

	if(0 != esp_ota_set_boot_partition(update_partition)) {
		return -1;
	}

    return 0;
}

int ota_verify(ota_flash_info_t *flash, int offset, size_t len)
{
	//todo
	return 0;
}
