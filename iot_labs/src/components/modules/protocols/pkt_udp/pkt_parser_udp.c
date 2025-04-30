#include "pkt_udp.h"

/* Abracadabra!
*
*  AES:
*  psk = MD5('SSL') = ea52c36203c5f99c3ce2442d531b1a22
*  key = MD5(psk + MAC)
*  IV = MD5(psk + key). Using a constant IV is risky.
*  packet's signature = MD5(packet_content(except hash))
*  
*  XOR:
*  TBD
*
*  
*
*  
*
*/

int pkt_parser_init(udp_packet_ctx_t *ctx, void* packet, size_t packet_len)
{
	udp_pkt_handler_t *handler  = (udp_pkt_handler_t*)packet;

	// packet
	uint16_t pkt_size = ntohs(handler->length);
	if(pkt_size > packet_len || pkt_size > UDP_PKT_SIZE_MAX || pkt_size < sizeof(udp_pkt_handler_t)) {
        LOG_ERROR("Invalid packet size.");
		return -1;
	}

	ctx->pkt_head = packet;
	ctx->pkt_size = pkt_size;

	// mac
	memcpy(ctx->mac, handler->mac, 6);

    // timestamp
	ctx->time_stamp = ntohl(handler->time_stamp);

	// type
	ctx->type = handler->type;

    // payload
    ctx->payload = ctx->pkt_head + sizeof(udp_pkt_handler_t);
    ctx->payload_len = ctx->pkt_size - sizeof(udp_pkt_handler_t);

	// hash
	memcpy(ctx->hash, handler->hash, 16);

	return 0;
}

int pkt_parser_end(udp_packet_ctx_t *ctx, udp_cipher_t *ciphers)
{
	// check signature
	{
		uint8_t hash[16] = {0};
		plt_md5(ctx->pkt_head + 20, ctx->pkt_size - 20, hash);
		if(0 != memcmp(ctx->hash, hash, 16)){
			LOG_ERROR("Invalid packet signature!");
			return -1;
		}
	}

	// decryption
	if(ctx->payload_len > 0) {
		uint8_t* output = malloc(ctx->payload_len);
		if(output){
			uint8_t iv[16];
			memcpy(iv, ciphers->iv, 16);
			plt_aes_decrypt(ctx->payload, ctx->payload_len, output, &ctx->payload_len, ciphers->key, 16, iv);
			memcpy(ctx->payload, output, ctx->payload_len);
			free(output);
		}
		else{
			LOG_ERROR("Decrypt buf malloc failed.");
			return -1;
		}

        // remove padding bytes
		ctx->payload_len -= plt_pkcs7_depadding(ctx->payload, ctx->payload_len, PKCS7_BASE);
	}

	return 0;
}
