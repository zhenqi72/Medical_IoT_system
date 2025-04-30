#include "crypto.h"

int get_random(uint8_t *buf, size_t len)
{
	unsigned int random = 0;

	for(size_t i=0, offset = 0; i < len; i++) {
		offset = i & 0x3;
		if(offset == 0)
			random = esp_random();

		buf[i] = (random >> (offset*8)) & 0xff;
	}

	return len;
}

void plt_md5(uint8_t *data, int len, uint8_t hash[16])
{
	struct MD5Context md5;

	MD5Init(&md5);
	MD5Update(&md5, data, len);
	MD5Final(hash, &md5);
}

size_t plt_pkcs7_padding(uint8_t* buf, size_t len, uint8_t modulus)
{
	uint8_t pad_byte;

	pad_byte = modulus - len%modulus;
	if(0 == pad_byte) {
        pad_byte = modulus;
    }

	if(buf) {
		for(int i = 0; i < pad_byte; i++)
			buf[len+i] = pad_byte;
	}

	return pad_byte;
}

size_t plt_pkcs7_depadding(uint8_t* buf, size_t len, uint8_t modulus)
{
	uint8_t *ptr_pad = buf + len - 1;
	uint8_t pad = *ptr_pad;

	if(pad > modulus || len < modulus) {
		return 0;
	}

	uint8_t pad_byte = pad;

	for(;pad > 0; pad--) {
		*ptr_pad-- = '\0';
    }

	return pad_byte;
}

// CBC
void plt_aes_encrypt(uint8_t* plaintxt, size_t plaintxt_len, uint8_t* ciptxt, size_t* ciptxt_len, uint8_t* key, size_t key_bytes, uint8_t* iv)
{
	esp_aes_context aes_ctx;
	esp_aes_setkey(&aes_ctx, key, key_bytes*8);
	esp_aes_crypt_cbc(&aes_ctx, ESP_AES_ENCRYPT, plaintxt_len, iv, (const uint8_t *)plaintxt, ciptxt);
    *ciptxt_len = plaintxt_len;
}

void plt_aes_decrypt(uint8_t* ciptxt, size_t ciptxt_len, uint8_t* plaintxt, size_t* plaintxt_len, uint8_t* key, size_t key_bytes, uint8_t* iv)
{
	esp_aes_context aes_ctx;
	esp_aes_setkey(&aes_ctx, key, key_bytes*8);
	esp_aes_crypt_cbc(&aes_ctx, ESP_AES_DECRYPT, ciptxt_len, iv, (const uint8_t *)ciptxt, plaintxt);
    *plaintxt_len = ciptxt_len;
}

void plt_xor_encrypt(uint8_t* plaintxt, size_t plaintxt_len, uint8_t* ciptxt, size_t* ciptxt_len, uint8_t* key)
{
	for(size_t i = 0; i < plaintxt_len; i++)
	{
		ciptxt[i] = plaintxt[i] ^ *key;
	}
	*ciptxt_len = plaintxt_len;
}

void plt_xor_decrypt(uint8_t* ciptxt, size_t ciptxt_len, uint8_t* plaintxt, size_t* plaintxt_len, uint8_t* key)
{
	for(size_t i = 0; i < ciptxt_len; i++)
	{
		plaintxt[i] = ciptxt[i] ^ *key;
	}
	*plaintxt_len = ciptxt_len;
}