#include "pkt_udp.h"


/* Abracadabra!
*
*  psk = MD5('SSL') = ea52c36203c5f99c3ce2442d531b1a22
*  key = MD5(psk + MAC)
*  IV = MD5(psk + key). Using s constant IV is always risky.
*  packet's signature = MD5(packet_content(except the first 20 bytes: type, length and hash))
*  
*/

static uint8_t psk[16] = {0xea, 0x52, 0xc3, 0x62, 0x03, 0xc5, 0xf9, 0x9c, 0x3c, 0xe2, 0x44, 0x2d, 0x53, 0x1b, 0x1a, 0x22};

void* pkt_create_udp(size_t payload_size)
{
    void* pbuf = NULL;
    size_t psize = PKT_REAL_SIZE(payload_size);

    if(psize < UDP_PKT_SIZE_MAX) {
		pbuf = malloc(psize);
        if(pbuf) {
			memset(pbuf, 0, psize);
		}else {
			LOG_ERROR("pbuf malloc failed.");
		}
    }else {
		LOG_ERROR("Invalid pkt_size %u", psize);
	}

    return pbuf;
}

void pkt_cipher_init(udp_cipher_t *ciphers, pkt_encrypt_t encrypt_type)
{
	//encryption type
	ciphers->encrypt_type = encrypt_type;

	// key
	uint8_t mac[6] = {0};
	get_mac(mac);

	uint8_t tmp_buf[32] = {0};
	memcpy(tmp_buf, psk, 16);
	memcpy(tmp_buf+16, mac, 6);
	plt_md5(tmp_buf, 22, ciphers->key);

	// IV
	memset(tmp_buf, 0, 32);
	memcpy(tmp_buf, psk, 16);
	memcpy(tmp_buf+16, ciphers->key, 16);
	plt_md5(tmp_buf, 32, ciphers->iv);
}

int pkt_wrapper_udp_init(udp_packet_ctx_t *ctx, uint8_t mac[6], uint32_t time_stamp, uint32_t type, uint8_t hash[16], void* packet)
{
	ctx->pkt_head = packet;
	ctx->pkt_size = sizeof(udp_pkt_handler_t);
	ctx->time_stamp = time_stamp;
	ctx->type = type;
	ctx->payload = packet + sizeof(udp_pkt_handler_t);
	ctx->payload_len = 0;

	if(mac) {
		memcpy(ctx->mac, mac, 6);
	}else {
		memset(ctx->mac, 0, 6);
	}

	if(hash) {
		memcpy(ctx->hash, hash, 16);
	}else {
		memset(ctx->hash, 0, 16);
	}

	return 0;
}

// fill payload
int pkt_wrapper_udp_update(udp_packet_ctx_t *ctx, size_t payload_len, void* payload)
{
	if(payload && payload_len) {
		memcpy(ctx->payload + ctx->payload_len, payload, payload_len);
	}

	ctx->payload_len += payload_len;

	return 0;
}

// wrap
int pkt_wrapper_udp_end(udp_packet_ctx_t* ctx, udp_cipher_t *ciphers)
{
	int ret = 0;
	udp_pkt_handler_t *handler = (udp_pkt_handler_t*)ctx->pkt_head;

	// AES-CBC
	if (ENCRYPT_TYPE_AES == ciphers->encrypt_type) {
		// type
		handler->type = ENCRYPT_TYPE_AES;

		// timestamp
		handler->time_stamp = htonl(ctx->time_stamp);

		// payload
		if(ctx->payload_len)
		{
			ctx->payload_len += plt_pkcs7_padding(ctx->payload, ctx->payload_len, PKCS7_BASE);

			uint8_t* output = malloc(ctx->payload_len);
			if(output){
				uint8_t iv[16] = {0};
				memcpy(iv, ciphers->iv, 16);
				plt_aes_encrypt(ctx->payload, ctx->payload_len, output, &ctx->payload_len, ciphers->key, 16, iv);
				memcpy(ctx->payload, output, ctx->payload_len);
				free(output);
			}
			else{
				ret = -1;
				LOG_ERROR("Encrypt buf malloc failed.");
				memset(ctx->payload, 0, ctx->payload_len);
			}
		}
		ctx->pkt_size += ctx->payload_len;

		// length
		handler->length = htons(ctx->pkt_size);
	}else{
		uint8_t* output = malloc(ctx->payload_len);
		plt_xor_encrypt(ctx->payload, *ctx->payload, output, &ctx->payload_len, ciphers->key);
		memcpy(ctx->payload, output, ctx->payload_len);
		free(output);
		ctx->pkt_size += ctx->payload_len;
		// length
		handler->length = htons(ctx->pkt_size);
	}

	// hash(packet's signature)
	plt_md5(ctx->pkt_head + 20, ctx->pkt_size - 20, ctx->hash);
	memcpy(handler->hash, ctx->hash, 16);

	return ret;
}
