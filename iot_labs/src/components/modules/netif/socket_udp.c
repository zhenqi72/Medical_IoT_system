#include "socket_udp.h"

void socket_unit_test(void)
{
	//Init
    socket_udp_t *udp = malloc(sizeof(socket_udp_t));
    if(udp != NULL)
        memset(udp, 0, sizeof(socket_udp_t));

    ip_info_t *local_ip = malloc(sizeof(ip_info_t));
    if(local_ip != NULL)
        memset(local_ip, 0, sizeof(ip_info_t));

    uint8_t *buf = malloc(1024);
    if(buf != NULL)
        memset(buf, 0, 1024);

    net_ip_port_t *ip_port = malloc(sizeof(net_ip_port_t));
    if(ip_port != NULL)
        memset(ip_port, 0, sizeof(net_ip_port_t));

    socket_udp_config_t udp_configs = {.port = 0, .timeout_read = 0, .timeout_write = 0};
    udp_configs.port = 12345;
    
    if( 0 == socket_udp_open(udp, &udp_configs) ) {
        plt_net_get_ip_info(local_ip);
        LOG_INFO("test socket opened.");
    } else{
        LOG_ERROR("test socket failed.");
    }

    //Recv
    int len = 0;
    while(1) {
		//read from
        len = socket_udp_read_from(udp, buf, sizeof(buf)-1, &ip_port->ip, &ip_port->port);

        if(len < 0){
        LOG_ERROR("socket read error, len<0");
            return;
        }

        if(len > 0){
            buf[len] = '\0';
            LOG_INFO("Recv content: %s", buf);
			//sendto
			socket_udp_write_to(udp, "This is a msg from your DEVICE", sizeof(buf)-1, &ip_port->ip, ip_port->port);
        }


    }
}

int socket_udp_open(socket_udp_t *udp, socket_udp_config_t* configs)
{
	LOG_DEBUG("opening");

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(configs->port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	udp->socket_fd = -1;
	udp->rw_mutex = NULL;
	udp->write_mutex = NULL;
	udp->read_mutex = NULL;

	if(configs->timeout_read > 0) {
		udp->timeout_read = configs->timeout_read;
	}else{
		udp->timeout_read = 100;
	}
	if(configs->timeout_write > 0) {
		udp->timeout_write = configs->timeout_write;
	}else{
		udp->timeout_write = 10*1000;
	}

	udp->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);

	if(udp->socket_fd < 0) {
		LOG_ERROR("socket error!" );
		goto err_exit;
	}

	if(bind(udp->socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		LOG_ERROR("connect error!" );
		goto err_exit;
	}

	struct timeval tv = {0, UDP_WR_TIMEOUT_INT*1000};
	if(setsockopt(udp->socket_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
		LOG_ERROR("setsockopt" );
		goto err_exit;
	}

	if(0 != mutex_create(&udp->read_mutex)  ||
	   0 != mutex_create(&udp->write_mutex) ||
	   0 != mutex_create(&udp->rw_mutex)) {
		goto err_exit;
	}

	return 0;

err_exit:
	if(udp->socket_fd >= 0) {
		close(udp->socket_fd);
		udp->socket_fd = -1;
	}
	if(udp->read_mutex) {
		mutex_delete(udp->read_mutex);
		udp->read_mutex = NULL;
	}
	if(udp->write_mutex) {
		mutex_delete(udp->write_mutex);
		udp->write_mutex = NULL;
	}
	if(udp->rw_mutex) {
		mutex_delete(udp->rw_mutex);
		udp->rw_mutex = NULL;
	}

	return -1;
}

static int _udp_recvfrom(socket_udp_t *udp, void* buf, size_t len, net_addr_t* ip, uint16_t *port)
{
	int ret = 0;

	if( 0 == mutex_get(udp->rw_mutex, WAIT_FOREVER) ) {

		struct sockaddr_in peer_addr;
		socklen_t addr_len = sizeof(peer_addr);

		ret = recvfrom(udp->socket_fd, buf, len, 0, (struct sockaddr*)&peer_addr, &addr_len);

		if(ret > 0) {
			*port = ntohs(peer_addr.sin_port);
			ip->s_addr = ntohl(peer_addr.sin_addr.s_addr);
		}
		if(ret < 0) {
			int error = 0;
			uint32_t optlen = sizeof(error);

			getsockopt(udp->socket_fd, SOL_SOCKET, SO_ERROR, &error, &optlen);
			switch(error) {
				case EINTR:
					ret = 0;
					break;
				default:
					break;
			}
		}
		mutex_put(udp->rw_mutex);
	}
	else{
		ret = -1;
	}

	return ret;
}

static int _udp_recvfrom_timeout(socket_udp_t *udp, void* pbuf, size_t len, net_addr_t* ip, uint16_t *port, uint32_t timeout_ms)
{
	int ret = 0;
	fd_set read_fds;
	struct timeval tv;

	FD_ZERO(&read_fds);
	FD_SET(udp->socket_fd, &read_fds);
	tv.tv_sec  = timeout_ms / 1000;
	tv.tv_usec = (timeout_ms % 1000) * 1000;

	ret = select(udp->socket_fd+1, &read_fds, NULL, NULL, timeout_ms==0 ? NULL:&tv);

	if (ret == 0) {
		return (0);
	}

	if (ret < 0) {
		if (errno == EINTR) {
			return (0);
		}
		return (ret);
	}

	return _udp_recvfrom(udp, pbuf, len, ip, port);
}

static int _udp_net_sendto(socket_udp_t *udp, const void* pbuf, size_t len, net_addr_t* ip, uint16_t port)
{
	int ret = 0;

	if(0 == mutex_get(udp->rw_mutex, WAIT_FOREVER)) {

		struct sockaddr_in peer_addr;
		peer_addr.sin_family = AF_INET;
		peer_addr.sin_port = htons(port);
		peer_addr.sin_addr.s_addr = htonl(ip->s_addr);

		ret = sendto(udp->socket_fd, pbuf, len, 0, (struct sockaddr *)&peer_addr, sizeof(peer_addr));

		if(ret < 0) {
			int error = 0;
			uint32_t optlen = sizeof(error);

			getsockopt(udp->socket_fd, SOL_SOCKET, SO_ERROR, &error, &optlen);
			switch(error) {
				case EINTR:
					ret = 0;
					break;
				default:
					LOG_ERROR("socket error = %d",  error);
					break;
			}
		}
		mutex_put(udp->rw_mutex);
	}
	else{
		ret = -1;
	}

	return ret;
}

int socket_udp_read_from(socket_udp_t *udp, void* pbuf, size_t len, net_addr_t* ip, uint16_t *port)
{
	int rlen = 0;

	if( 0 == mutex_get(udp->read_mutex, udp->timeout_read)) {
		rlen = _udp_recvfrom_timeout(udp, pbuf, len, ip, port, udp->timeout_read);
		mutex_put(udp->read_mutex);
	}

	return rlen;
}

int socket_udp_write_to(socket_udp_t *udp, const void* pbuf, size_t len, net_addr_t* ip, uint16_t port)
{
	int ret = -1;
	size_t written_len = 0;

	if( 0 == mutex_get(udp->write_mutex, WAIT_FOREVER) ){

		uint32_t write_timeout = 0;
		do{
			ret = _udp_net_sendto(udp, pbuf + written_len, len - written_len, ip, port);

			if(ret > 0){
				written_len += ret;
				if(written_len != len){
					LOG_WARN("upd_net_sendto %u written, %u need, returned %d", ret, written_len, len);
				}
			}
			else if(ret == 0){
				write_timeout += UDP_WR_TIMEOUT_INT;
				if(write_timeout >= udp->timeout_write){
					LOG_ERROR("udp_net_sendto timeout %ums, %u written, %u need", write_timeout, written_len, len);
					break;
				}
				else{
					LOG_WARN("udp_net_sendto %u written, %u need, returned %d", ret, written_len, len);
				}
			}
			else{
				LOG_ERROR("upd_net_sendto %u written, %u need, returned -0x%x", -ret, written_len, len);
				break;
			}

		}while(written_len < len);

		mutex_put(udp->write_mutex);
	}

	if(written_len < len)
		return ret;

	return written_len;
}

void socket_udp_close(socket_udp_t *udp)
{
	close(udp->socket_fd);
	udp->socket_fd = -1;

	if(udp->read_mutex){
		mutex_delete(udp->read_mutex);
		udp->read_mutex = NULL;
	}
	if(udp->write_mutex){
		mutex_delete(udp->write_mutex);
		udp->write_mutex = NULL;
	}
	if(udp->rw_mutex){
		mutex_delete(udp->rw_mutex);
		udp->rw_mutex = NULL;
	}

}
