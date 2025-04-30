#ifndef _PACKET_UDP_H_
#define _PACKET_UDP_H_

#include "labs_defines.h"
#include "labs_platform.h"


#define PKT_TYPE_INVALID        0xff
#define PKT_TYPE_RPC            0x01

typedef enum{
	ENCRYPT_TYPE_NONE = 0,
 	ENCRYPT_TYPE_AES,
	ENCRYPT_TYPE_XOR
}pkt_encrypt_t;

typedef struct{
	pkt_encrypt_t encrypt_type;
	uint8_t key[16];
	uint8_t iv[16];
}udp_cipher_t;

#define TS_EXIST        		1

#if TS_EXIST
typedef struct{
	size_t pkt_size;
	uint8_t mac[6];
	uint32_t time_stamp;
	uint32_t type;
	uint8_t hash[16];
	uint8_t *pkt_head;
	uint8_t *payload;
	size_t payload_len;
}udp_packet_ctx_t;

struct udp_pkt_handler{
	uint16_t type;
	uint16_t length;
	uint8_t hash[16];
	uint8_t mac[6];
	uint32_t time_stamp; // Remove for replay attacks
}__attribute__ ((__packed__));
typedef struct udp_pkt_handler udp_pkt_handler_t;

#else
typedef struct{
	size_t pkt_size;
	uint8_t mac[6];
	uint32_t type;
	uint8_t hash[16];
	uint8_t *pkt_head;
	uint8_t *payload;
	size_t payload_len;
}udp_packet_ctx_t;

struct  __attribute__ ((__packed__)) udp_pkt_handler{
	uint16_t type;
	uint16_t length;
	uint8_t mac[6];
	uint8_t hash[16];
};

typedef struct udp_pkt_handler udp_pkt_handler_t;

#endif

#define PKCS7_BASE				        16
#define UDP_PKT_SIZE_MAX 		        1400 //MTU
#define UDP_PKT_PAYLOAD_SIZE_MAX		(PACKET_SIZE_MAX - sizeof(udp_pkt_handler_t))

#define PKT_REAL_SIZE(payload_size)     ( sizeof(udp_pkt_handler_t) + payload_size + (payload_size ? plt_pkcs7_padding(NULL, payload_size, PKCS7_BASE) : 0) )            

/**
 * @brief Create a UDP packet
 * 
 * @param payload_size Size of the payload in the packet
 * @return void* 
 */
void* pkt_create_udp(size_t payload_size);
/**
 * @brief Create a cipher
 * 
 * @param ciphers Pointer to the data to create the cipher from
 * @param encrypt_type Type of encryption to use
 */
void pkt_cipher_init(udp_cipher_t *ciphers, pkt_encrypt_t encrypt_type);
/**
 * @brief Initialize a wrapper for a UDP packet
 * 
 * @param ctx Pointer to the created wrapper data
 * @param mac MAC Address
 * @param time_stamp Time stamp for the packet
 * @param type Type of encrytion for the packet
 * @param hash Hash for the packet
 * @param packet The data packet to send
 * @return int 
 */
int pkt_wrapper_udp_init(udp_packet_ctx_t *ctx, uint8_t mac[6], uint32_t time_stamp, uint32_t type, uint8_t hash[16], void* packet);
/**
 * @brief Update payload in a UDP packet
 * 
 * @param ctx Wrapped packet
 * @param payload_len Length of the new payload
 * @param payload Pointer to the payload
 * @return int 
 */
int pkt_wrapper_udp_update(udp_packet_ctx_t *ctx, size_t payload_len, void* payload);
/**
 * @brief Finish wrapping and encrypting the packet
 * 
 * @param ctx Pointer to the wrapped packet
 * @param ciphers The information to use to encrypt the packet
 * @return int 
 */
int pkt_wrapper_udp_end(udp_packet_ctx_t* ctx, udp_cipher_t *ciphers);
/**
 * @brief Initialize the packet parser
 * 
 * @param ctx Pointer to the wrapped packet
 * @param packet Pointer to the packet
 * @param packet_len Size of the packet
 * @return int 
 */
int pkt_parser_init(udp_packet_ctx_t *ctx, void* packet, size_t packet_len);
/**
 * @brief End parsing the packet and decrypt
 * 
 * @param ctx Pointer to the wrapped packet
 * @param ciphers Pointer to the cyphers to decrypt the packet
 * @return int 
 */
int pkt_parser_end(udp_packet_ctx_t *ctx, udp_cipher_t *ciphers);


#endif
