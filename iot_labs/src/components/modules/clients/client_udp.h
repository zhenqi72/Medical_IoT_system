#ifndef __CLIENT_UDP_H__
#define __CLIENT_UDP_H__

#include "labs_platform.h"
#include "socket_udp.h"
#include "pkt_udp.h"
#include "rpc_handler.h"

#define DEFAULT_LOCAL_PORT    20123
#define DEFAULT_QUEUE_LEN	  5
#define DEFAULT_UDP_CLIENT_TASK_STACK	  4096
/********************** Client Workflow ***************************
* 
* 				 			(Error occurs ) -> SHUTDOWN
* IDLE -> INIT -> ACTIVE ->
* 				 			(Regular exit) -> DEINIT -> IDLE
*
******************************************************************/
typedef enum {
	CLIENT_UDP_SHUTDOWN = 0,
	CLIENT_UDP_IDLE,
	CLIENT_UDP_INIT,
	CLIENT_UDP_ACTIVE,
	CLIENT_UDP_DEINIT
}client_udp_state_t;

#define CLIENT_MSG_INIT			0x00
#define CLIENT_MSG_DEINIT		0x01
#define CLIENT_MSG_EXIT			0x02
#define CLIENT_MSG_INVALID		0xff

typedef struct{
	uint8_t type;
	size_t argc;
	void *argv;
}client_udp_msg_t;

typedef struct client_udp{
	socket_udp_t udp;
	uint16_t local_port;
	thread_handle_t thread;
	queue_handle_t msg_queue;
	client_udp_state_t state;

	rpc_handler_t *rpc_handler;

	uint8_t mac[6];
	udp_cipher_t cipher_ctx;
	uint8_t buf[UDP_PKT_SIZE_MAX+1];
	net_ip_port_t last_ip_port;//last packet's source IP, save for ack

	ip_info_t local_ip_info;
	ip_info_t host_info;
}client_udp_t;

#define CLIENT_UDP_MODE_OFFLINE      0
#define CLIENT_UDP_MODE_ONLINE		 1

/**
 * @brief Initializes a UDP client
 * @param client Handle to the client parameters to init
 * @param encrypt_type Type of encrytion to use
 */
int client_udp_init(client_udp_t* client, pkt_encrypt_t encrypt_type);
/**
 * @brief Deinitialize a UDP client
 * @param client Handle to the client to deinitialize
 * @return null
 */
void client_udp_deinit(client_udp_t* client);
/**
 * @brief Set the UDP mode of the client
 * @param client Handle to the client to set the mode of
 * @param mode Mode to set the client to
 * @return 0 on success -1 on failure
 * @warning This will never fail
 */
int client_udp_set_mode(client_udp_t* client, uint8_t mode);

#endif
