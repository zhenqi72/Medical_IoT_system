#include "socket_tcp.h"

int socket_tcp_open(socket_tcp_t *tcp, socket_tcp_config_t* configs)
{
	LOG_DEBUG("opening");

	struct sockaddr_in ser_addr;
	bzero(&ser_addr, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(configs->ip_port.port);
	ser_addr.sin_addr.s_addr = htonl(configs->ip_port.ip.s_addr);

	tcp->socket_fd = -1;
	tcp->read_mutex = NULL;
	tcp->write_mutex = NULL;
	tcp->rw_mutex = NULL;
	if(configs->timeout_read > 0){
		tcp->timeout_read = configs->timeout_read;
	}
	else{
		tcp->timeout_read = 100;
	}
	if(configs->timeout_write > 0){
		tcp->timeout_write = configs->timeout_write;
	}
	else{
		tcp->timeout_write = 10*1000;
	}

	tcp->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(tcp->socket_fd < 0){
		LOG_ERROR("socket error!" );
		goto err_exit;
	}
	int errVal = connect(tcp->socket_fd, (struct sockaddr *)&ser_addr, sizeof(ser_addr));
	if( errVal != 0 ){
		LOG_INFO("ErrVal %d", errVal);
		LOG_ERROR("connect error!" );
		goto err_exit;
	}

    struct timeval tv = {0, TCP_WR_TIMEOUT_INT*1000};
    if(setsockopt(tcp->socket_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0){
        LOG_ERROR("setsockopt error!" );
        goto err_exit;
    }

    int no_delay = 1;

    if(setsockopt(tcp->socket_fd, IPPROTO_TCP, TCP_NODELAY, (void*)&no_delay, sizeof(no_delay)) < 0){
        LOG_WARN("setsockopt TCP_NODELAY error!" );
    }

	if(0 != mutex_create(&tcp->read_mutex)  ||
	   0 != mutex_create(&tcp->write_mutex) ||
	   0 != mutex_create(&tcp->rw_mutex)) {
		goto err_exit;
	}

	LOG_DEBUG("opened" );

	return 0;

err_exit:
	if(tcp->socket_fd >= 0){
		close(tcp->socket_fd);
		tcp->socket_fd = -1;
	}
	if(tcp->read_mutex){
		mutex_delete(tcp->read_mutex);
		tcp->read_mutex = NULL;
	}
	if(tcp->write_mutex){
		mutex_delete(tcp->write_mutex);
		tcp->write_mutex = NULL;
	}
	if(tcp->rw_mutex){
		mutex_delete(tcp->rw_mutex);
		tcp->rw_mutex = NULL;
	}

	LOG_ERROR("open failed!" );

	return -1;
}

static int _tcp_net_recv(socket_tcp_t *tcp, void* pbuf, size_t len)
{
	int ret = 0;

	if( 0 == mutex_get(tcp->rw_mutex, WAIT_FOREVER) ){
		ret = recv(tcp->socket_fd, pbuf, len, 0);
		if(ret < 0){
			int error = 0;
			uint32_t optlen = sizeof(error);

			getsockopt(tcp->socket_fd, SOL_SOCKET, SO_ERROR, &error, &optlen);
			switch (error) {
                case EINTR:
                    ret = 0;
                    break;
                default:
                    LOG_ERROR("socket error = %d",  error);
                    break;
			}
		}

		mutex_put(tcp->rw_mutex);
	}
	else{
		ret = -1;
	}

	return ret;

}

static int _tcp_net_recv_timeout(socket_tcp_t *tcp, void* pbuf, size_t len, uint32_t timeout_ms)
{
	int ret = 0;
	fd_set read_fds;
	struct timeval tv;

	FD_ZERO( &read_fds );
	FD_SET( tcp->socket_fd, &read_fds );
	tv.tv_sec  = timeout_ms / 1000;
	tv.tv_usec = ( timeout_ms % 1000 ) * 1000;

	ret = select( tcp->socket_fd + 1, &read_fds, NULL, NULL, timeout_ms == 0 ? NULL : &tv );

	if ( ret == 0 ) {
		return ( 0 );
	}

	if ( ret < 0 ) {
		if ( errno == EINTR ) {
			return ( 0 );
		}
		return ( ret );
	}

	return _tcp_net_recv(tcp, pbuf, len);
}


static int _tcp_net_send(socket_tcp_t *tcp, const void* pbuf, size_t len)
{
	int ret = 0;

	if( 0 == mutex_get(tcp->rw_mutex, WAIT_FOREVER) ){

		ret = send(tcp->socket_fd, pbuf, len, 0);

		if(ret < 0){
			int error = 0;
			uint32_t optlen = sizeof(error);

			getsockopt(tcp->socket_fd, SOL_SOCKET, SO_ERROR, &error, &optlen);

			switch ( error ) {
			case EINTR:
				ret = 0;
				break;
			default:
				LOG_ERROR("socket error = %d",  error);
				break;
			}
		}

		mutex_put(tcp->rw_mutex);
	}
	else{
		ret = -1;
	}

	return ret;
}

int socket_tcp_read_from(socket_tcp_t *tcp, void* pbuf, size_t len)
{
	int readed_len = 0;

	if( 0 == mutex_get(tcp->read_mutex, tcp->timeout_read) ){

		readed_len = _tcp_net_recv_timeout(tcp, pbuf, len, tcp->timeout_read);

		mutex_put(tcp->read_mutex);
	}

	return readed_len;
}

int socket_tcp_write_to(socket_tcp_t *tcp, const void* pbuf, size_t len)
{
	int ret = -1;
	size_t written_len = 0;

	if( 0 == mutex_get(tcp->write_mutex, WAIT_FOREVER) ){

		uint32_t timeout_write = 0;
		do{
			ret = _tcp_net_send(tcp, pbuf + written_len, len - written_len);

			if(ret > 0){
				written_len += ret;
				if(written_len != len){
					LOG_WARN("tcp_net_send returned %d, %u written, %u need", ret, written_len, len);
				}
			}
			else if(ret == 0){
				timeout_write += TCP_WR_TIMEOUT_INT;
				if(timeout_write >= tcp->timeout_write){
					LOG_ERROR("tcp_net_send timeout %ums, %u written, %u need", timeout_write, written_len, len);
					break;
				}
				else{
					LOG_WARN("tcp_net_send timeout %ums, %u written, %u need", timeout_write, written_len, len);
				}
			}
			else{
				LOG_ERROR("tcp_net_send returned -0x%x, %u written, %u need", -ret, written_len, len);
				break;
			}

		}while(written_len < len);

		mutex_put(tcp->write_mutex);
	}

	if(written_len < len)
		return ret;

	return written_len;
}

void socket_tcp_close(socket_tcp_t *tcp)
{
	close(tcp->socket_fd);
	tcp->socket_fd = -1;

	if(tcp->read_mutex){
		mutex_delete(tcp->read_mutex);
		tcp->read_mutex = NULL;
	}
	if(tcp->write_mutex){
		mutex_delete(tcp->write_mutex);
		tcp->write_mutex = NULL;
	}
	if(tcp->rw_mutex){
		mutex_delete(tcp->rw_mutex);
		tcp->rw_mutex = NULL;
	}

	LOG_DEBUG("closed" );
}

