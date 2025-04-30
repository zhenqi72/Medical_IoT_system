
#ifndef __NET_H__
#define __NET_H__

#include "port.h"

#define IEEEtypes_SSID_SIZE				32
#define WLAN_PSK_MAX_LENGTH				64


typedef struct net_addr {
	uint32_t s_addr;
}net_addr_t;

#define NET_ADDR_INIT_VALUE				\
{										\
	.s_addr = "127.0.0.1"				\
}

typedef struct {
	net_addr_t ip;
	uint16_t port;
}net_ip_port_t;

#define NET_IP_PORT_INIT_VALUE			\
{										\
	.ip = NET_ADDR_INIT_VALUE,			\
	.port = 1337						\
}

typedef struct {
	net_addr_t ip;
	net_addr_t netmask;
	net_addr_t gateway;
}ip_info_t;

#define IP_INFO_INIT_VALUE				\
{										\
	.ip = NET_ADDR_INIT_VALUE,			\
	.netmask = NET_ADDR_INIT_VALUE,		\
	.gateway = NET_ADDR_INIT_VALUE			\
}

typedef struct {
	uint8_t ssid[33];         			/**< SSID of target AP*/
    uint8_t password[65];     			/**< password of target AP*/
    uint8_t bssid[6];         			/**< MAC address of target AP*/
	int8_t  channel;                    /**< channel of AP */
	int8_t  rssi;                       /**< signal strength of AP */
}net_wifi_ap_info_t;

/**
 * @brief Changes the byte order from host to network
 * @param n bytes to be changed
 * @retval n in network byte order
 */
uint64_t plt_net_htonll(uint64_t n);
/**
 * @brief Changes the byte order from network to host
 * @param n bytes to be changed
 * @retval n in host byte order
 */
uint64_t plt_net_ntohll(uint64_t n);
/**
 * @brief Convert numeric IP address into decimal dotted ASCII representation
 * @param paddr A pointer to the address structure
 * @retval A character pointer to the string
 */
char *plt_net_htoa(net_addr_t* paddr);
/**
 * @brief Network address to string
 * @param paddr Pointer to the address
 * @param buf Pointer to the buffer to write the string to
 * @param buf_len Size of the output buffer
 * @return If the buffer is too small it returns the size needed
 */
int plt_net_htoa_buf(net_addr_t* paddr, char* buf, int buf_len);

/**
 * @brief Turns the network name into a string
 * @param name Name of host to get address from
 * @param addr Pointer to struct to hold addresses
 * @return 0 on success -1 on failure
 */
int plt_net_name2ip(const char *name, net_addr_t *addr);
/**
 * @brief character IP to number
 * @param ip Pointer to IP represented as string
 * @return The IP represented as a number
 */
uint32_t plt_net_ip2n(const char* ip);

/**
 * @brief Check if the network is online
 * @param null
 * @return True if network is offline false otherwise
 */
bool plt_net_is_offline(void);

/**
 * @brief Get the current network's IP information
 * @param ip_info Handle to store the retrieved IP info to
 * @return 0 on success -1 on failure
 */
int plt_net_get_ip_info(ip_info_t *ip_info);
/**
 * @brief Get the current network's access point information
 * @param ap_info Handle to store the retrieved AP info to
 * @return 0 on success -1 on failure
 */
int plt_net_get_ap_info(net_wifi_ap_info_t *ap_info);

#endif

