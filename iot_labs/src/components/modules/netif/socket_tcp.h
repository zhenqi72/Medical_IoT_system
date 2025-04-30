#ifndef __SOCKET_TCP_H__
#define __SOCKET_TCP_H__

#include "labs_platform.h"

#define TCP_TIMEOUT_MAX	    (10*1000)
#define TCP_WR_TIMEOUT_INT	(1000)

typedef struct{
	net_ip_port_t ip_port;
	uint32_t timeout_read;
	uint32_t timeout_write;
}socket_tcp_config_t ;

#define TCP_CONFIG_INIT_VALUE \
{							  \
    .ip_port = NET_IP_PORT_INIT_VALUE, \
	.timeout_read = 0,		  \
	.timeout_write = 0		  \
}

typedef struct socket_tcp {
	int socket_fd;
	mutex_handle_t rw_mutex;
    mutex_handle_t read_mutex;
	mutex_handle_t write_mutex;
	uint32_t timeout_read; //ms
	uint32_t timeout_write; //ms
}socket_tcp_t;

/**
 * @brief Open a TCP socket
 * @param tcp Pointer the the socket to open
 * @param cfg Pointer to the configuration for that socket
 * @return 0 on success -1 on failure
 */
int socket_tcp_open(socket_tcp_t *tcp, socket_tcp_config_t* cfg);
/**
 * @brief Read data from the TCP socket
 * @param tcp Pointer to the socket to read from
 * @param pbuff Buffer to read data into
 * @param len Size of the buffer to read data into
 * @return 0 on success -1 on failure
 */
int socket_tcp_read_from(socket_tcp_t *tcp, void* pbuf, size_t len);
/**
 * @brief Write data to TCP socket
 * @param tcp Pointer to the socket to write to
 * @param pbuff Pointer to the buffer holding the data to write
 * @param len Length of the buffer
 * @return 0 on success -1 on failure
 */
int socket_tcp_write_to(socket_tcp_t *tcp, const void* pbuf, size_t len);
/**
 * @brief Close TCP socket
 * @param tcp Pointer to the TCP socket to close
 * @return null
 */
void socket_tcp_close(socket_tcp_t *tcp);
#endif
