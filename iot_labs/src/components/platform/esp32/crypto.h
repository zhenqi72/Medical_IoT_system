#ifndef _CRYPTO_H_
#define _CRYPTO_H_

#include "port.h"
/**
 * @brief Generate a random integer
 * @warning This uses the ESP32 `esp_random` to generate a random number. This means that if the RF system is not running the number should be treated as PRNG.
 * @see esp_random
 * @param[out] buf A pointer to the buffer of random uint8_t
 * @param[in] len the length of the buffer or how many random numbers you would like to be generated
 * @return Length of generated buffer
 */
int get_random(uint8_t *buf, size_t len);
/**
 * @brief Gnerate a MD5 hash
 * @see MD5_hash.h
 * @param data The data to be hashed
 * @param len The length of the data to be hashed
 * @param[out] hash The hashed data
 */
void plt_md5(uint8_t *data, int len, uint8_t hash[16]);
/**
 * @brief Padds a buffer
 * @param buf A pointer to the buffer to pad
 * @param len The length of the buffer
 * @param modulus The base of the padded packet
 * @return TODO
 */
size_t plt_pkcs7_padding(uint8_t* buf, size_t len, uint8_t modulus);
/**
 * @brief Depadds a buffer
 * @param buf A pointer to the buffer to depad
 * @param len The length of the buffer
 * @param modulus The base of the padded packet
 * @return TODO
 */
size_t plt_pkcs7_depadding(uint8_t* buf, size_t len, uint8_t modulus);
/**
 * @brief encrypts data using AES
 * @param plaintxt A pointer to the data to be encrypted
 * @param plaintxt_len The size of the data to be encrypted
 * @param[out] ciptxt The encrypted data
 * @param[out] cpitxt_len The length of the encrytped data
 * @param key The key to encrypt the data
 * @param key_bytes The number of bytes in the key
 * @param iv Initialization vector
 */
void plt_aes_encrypt(uint8_t* plaintxt, size_t plaintxt_len, uint8_t* ciptxt, size_t* ciptxt_len, uint8_t* key, size_t key_bytes, uint8_t* iv);
/**
 * @brief derypts data using AES
 * @param ciptxt A pointer to the data to be decrypted
 * @param cpitxt_len The length of the data to be decrypted
 * @param[out] plaintxt The decrypted data
 * @param[out] plaintxt_len The length of the decrypted data
 * @param key The key to decrypt the data
 * @param key_bytes The number of bytes in the key
 * @param iv Initialization vector
 */
void plt_aes_decrypt(uint8_t* ciptxt, size_t ciptxt_len, uint8_t* plaintxt, size_t* plaintxt_len, uint8_t* key, size_t key_bytes, uint8_t* iv);
/**
 * @brief Encrypts data using XOR
 * @param plaintxt A pointer to the data to be encrypted
 * @param plaintxt_len The size of the data to be encrypted
 * @param ciptxt ciptxt The encrypted data
 * @param ciptxt_len cpitxt_len The length of the encrytped data
 * @param key The key to encrypt the data
 */
void plt_xor_encrypt(uint8_t* plaintxt, size_t plaintxt_len, uint8_t* ciptxt, size_t* ciptxt_len, uint8_t* key);
/**
 * @brief Decrypts the data with an XOR key
 * @param ciptxt A pointer to the data to be decrypted
 * @param ciptxt_len The length of the data to be decrypted
 * @param plaintxt The decrypted data
 * @param plaintxt_len The length of the decrypted data
 * @param key The key to encrypt the data
 */
void plt_xor_decrypt(uint8_t* ciptxt, size_t ciptxt_len, uint8_t* plaintxt, size_t* plaintxt_len, uint8_t* key);
#endif