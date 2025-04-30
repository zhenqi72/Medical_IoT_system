#ifndef __I2C_H_
#define __I2C_H_
/**
 * Includes
 */
#include "port.h"
#include "debug_log.h"

/**
 * @brief Initilize a i2c peripheral
 * @param num The peripheral to initialize
 * @return 0 on success -1 on failure
 */
int i2c_init(int num);
/**
 * @brief Initilize a i2c peripheral
 * @param num The i2c peripheral to use
 * @param buf Pointer to data to receive
 * @param len Length of buffer
 * @param timeout Maximum amount of time to receive
 * @return 0 on success -1 on failure
 */
int i2c_read(int num, unsigned char *buf, int len, int timeout);
/**
 * @brief Initilize a i2c peripheral
 * @param num The i2c peripheral to use
 * @param buf Pointer to data to send
 * @param len Length of buffer
 * @return 0 on success -1 on failure
 */
void i2c_write(int num, const unsigned char *buf, int len);
/**
 * @brief Uninstall i2c driver
 * @param num The peripheral to uninstall
 * @return 0 on success -1 on failure
 */
int i2c_delete(int num);

#endif