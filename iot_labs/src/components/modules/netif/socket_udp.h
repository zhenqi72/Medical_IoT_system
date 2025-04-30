#ifndef __SOCKET_UDP_H__
#define __SOCKET_UDP_H__

#include "labs_platform.h"

#define UDP_TIMEOUT_MAX		(10*1000)
#define UDP_WR_TIMEOUT_INT	(1000)

typedef struct{
	uint16_t port;
	uint32_t timeout_read;
	uint32_t timeout_write;
}socket_udp_config_t;

typedef struct socket_udp {
	int socket_fd;
	mutex_handle_t rw_mutex;
    mutex_handle_t read_mutex;
	mutex_handle_t write_mutex;
	uint32_t timeout_read; //ms
	uint32_t timeout_write; //ms
}socket_udp_t;

void socket_unit_test(void);
/**
 * @brief Open a UDP socket
 * @param udp Pointer the the socket to open
 * @param cfg Pointer to the configuration for that socket
 * @return 0 on success -1 on failure
 */
int socket_udp_open(socket_udp_t *udp, socket_udp_config_t* cfg);
/**
 * @brief Read data from the UDP socket
 * @param udp Pointer to the socket to read from
 * @param pbuff Buffer to read data into
 * @param len Size of the buffer to read data into
 * @param ip IP to read
 * @param port Pointer to the port to read from
 * @return 0 on success -1 on failure
 */
int socket_udp_read_from(socket_udp_t *udp, void* pbuf, size_t len, net_addr_t* ip, uint16_t *port);
/**
 * @brief Write data from the UDP socket
 * @param udp Pointer to the socket to read from
 * @param pbuff Buffer to read data into
 * @param len Size of the buffer to read data into
 * @param ip IP to read
 * @param port Pointer to the port to read from
 * @return 0 on success -1 on failure
 */
int socket_udp_write_to(socket_udp_t *udp, const void* pbuf, size_t len, net_addr_t* ip, uint16_t port);
/**
 * @brief Close UDP socket
 * @param tcp Pointer to the UDP socket to close
 * @return null
 */
void socket_udp_close(socket_udp_t *udp);

#endif
