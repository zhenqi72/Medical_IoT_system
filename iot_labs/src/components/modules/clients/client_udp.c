#include "client_udp.h"


static int _client_set_msg(client_udp_t* client, uint8_t msg_type, void* argv, size_t argc, uint32_t timeout)
{
    client_udp_msg_t msg = {
        .type = msg_type,
        .argv = NULL,
        .argc = argc
    };

    if(argv && argc) {
        msg.argv = malloc(argc);
        if(NULL == msg.argv) {
            goto error_exit;
        }
        memcpy(msg.argv, argv, argc);
        msg.argc = argc;
    }

    if(0 != queue_send(client->msg_queue, &msg, timeout)){
		if(msg.argv){
			free(msg.argv);
		}
		goto error_exit;
	}

	LOG_DEBUG("set msg type=%d, arg_len=%d", msg_type, argc);

	return 0;

error_exit:

	LOG_ERROR("set msg type=%d, arg_len=%d", msg_type, argc);

	return -1;
}

static int _client_get_msg(client_udp_t* client, client_udp_msg_t *msg, uint32_t timeout)
{
	msg->type = CLIENT_MSG_INVALID;
	msg->argc = 0;
	msg->argv = NULL;

	if(0 == queue_recv(client->msg_queue, msg, timeout)) {
		LOG_DEBUG("Client udp get ctrl msg=%d, argc=%d", msg->type, msg->argc);
		return 0;
	}

	return -1;
}

static void _client_free_msg(client_udp_msg_t *msg)
{
	if(msg->argv) {
		free(msg->argv);
		msg->argv = NULL;
	}
	msg->argc = 0;
}

int client_udp_set_mode(client_udp_t* client, uint8_t mode)
{
    switch(mode) {
        case CLIENT_UDP_MODE_OFFLINE:
            _client_set_msg(client, CLIENT_MSG_DEINIT, NULL, 0, WAIT_FOREVER);
            break;
        case CLIENT_UDP_MODE_ONLINE:
            _client_set_msg(client, CLIENT_MSG_INIT, NULL, 0, WAIT_FOREVER);
            break;
        default:
            break;
    }
    return 0;
}

static void _client_packet_handler(client_udp_t* client, uint8_t *pkt_buf, uint16_t pkt_len)
{
    LOG_DEBUG("Client UDP: Packet received.");
    //client_udp_sendto(client, &client->last_ip_port, PKT_TYPE_RPC, "This is an ACK.", 16);
    switch(client->cipher_ctx.encrypt_type) {
        case ENCRYPT_TYPE_AES:
            LOG_INFO("Decrypting AES pkt...");
            udp_packet_ctx_t parser_ctx;
            if(0 == pkt_parser_init(&parser_ctx, pkt_buf, (size_t)pkt_len)) {
                //Verify MAC
                if(!memcmp(client->mac, parser_ctx.mac, 6))
                {
                    if(0 == pkt_parser_end(&parser_ctx, &client->cipher_ctx))
                    {
                        rpc_handler_put(client->rpc_handler, parser_ctx.payload, parser_ctx.payload_len, RPC_JSON);
                    }
                }
                else{
                    LOG_INFO("Packet dropped: Wrong MAC address.");
                }
            }
            break;
        case ENCRYPT_TYPE_XOR:
            //todo
            break;
        case ENCRYPT_TYPE_NONE:
            //todo
            break;
        default:
            LOG_ERROR("Unknown ENCRYPT_TYPE");
            break;
    }
}

//ip: host order
static int _client_udp_sendto(client_udp_t* client, net_ip_port_t* ip_port, uint8_t pkt_type, void* payload, size_t plen)
{
	if(client->state != CLIENT_UDP_ACTIVE){
		LOG_WARN("Client is not ACTIVE, sendto failed");
		return -1;
	}

	int ret = 0;
	size_t pkt_len = 0;
	uint8_t* pkt_buf = NULL;
	udp_cipher_t *cipher = &client->cipher_ctx;

	//todo:ts sync
    //uint32_t ts = ;

	switch(pkt_type){
	case PKT_TYPE_RPC:
		if(NULL != (pkt_buf = malloc(PKT_REAL_SIZE(plen)))) {
			udp_packet_ctx_t ctx;
			pkt_wrapper_udp_init(&ctx, client->mac, PKT_TYPE_RPC, 0, NULL, pkt_buf);//todo:ts
			pkt_wrapper_udp_update(&ctx, plen, payload);
			pkt_wrapper_udp_end(&ctx, cipher);
			pkt_len = ctx.pkt_size;
		}
		else{
			LOG_ERROR("Packet malloc failed.");
			ret = -1;
		}
		break;
	default:
		LOG_ERROR("Invalid packet type %u", pkt_type);
		ret = -1;
		break;
	}

	if(pkt_buf && pkt_len > 0){
		if(pkt_len != socket_udp_write_to(&client->udp, pkt_buf, pkt_len, &ip_port->ip, ip_port->port)){
			LOG_WARN("Packet send error!");
			ret = -1;
		}
        else
            LOG_DEBUG("Client sent pkt to %u", ip_port->ip.s_addr);
	}

	if(pkt_buf){
		free(pkt_buf);
	}

	return ret;
}

static int _client_udp_ack(client_udp_t* client, uint8_t pkt_type, void* payload, size_t plen)
{
    return _client_udp_sendto(client, &client->last_ip_port, pkt_type, payload, plen);
}

static function_cb_t client_udp_main(void * arg)
{
    int len = 0;
    client_udp_t* client = (client_udp_t*)arg;
    client_udp_msg_t msg = {
            .type = 0xff,
            .argv = NULL,
            .argc = 0
        };

    while(CLIENT_UDP_SHUTDOWN != client->state) {
    	switch(client->state){
            case CLIENT_UDP_IDLE:
                LOG_DEBUG("Client State: IDLE");
                if(0 == _client_get_msg(client, &msg, WAIT_FOREVER)){
                    if(CLIENT_MSG_EXIT == msg.type){
                        client->state = CLIENT_UDP_SHUTDOWN;
                    }
                    else if(CLIENT_MSG_INIT == msg.type){
                        client->state = CLIENT_UDP_INIT;
                    }
                    else{
                        LOG_WARN("ctrl msg[%d] droped!", msg.type);
                    }
                    _client_free_msg(&msg);
                }

                break;

            case CLIENT_UDP_INIT:
                LOG_DEBUG("Client State: INIT");
                socket_udp_config_t udp_configs = {.port = 0, .timeout_read = 0, .timeout_write = 0};
                udp_configs.port = client->local_port;

                if( 0 == socket_udp_open(&client->udp, &udp_configs) ){
                    plt_net_get_ip_info(&client->local_ip_info);
                    client->state = CLIENT_UDP_ACTIVE;
                    LOG_INFO("Client UDP activated.");
                } else{
                    client->state = CLIENT_UDP_IDLE;
                    LOG_ERROR("Client UDP init fail.");
                }
                break;

            case CLIENT_UDP_ACTIVE:
                //Receiving packet
                len = socket_udp_read_from(&client->udp, client->buf, sizeof(client->buf)-1, &client->last_ip_port.ip, &client->last_ip_port.port);
                
                if(len < 0){
                    client->state = CLIENT_UDP_DEINIT;
                    break;
                }

                if(len > 0){
                    LOG_INFO("socket recv len = %d", len);
                    client->buf[len] = '\0';
                    //Handle packet
                    _client_packet_handler(client, client->buf, len);
                }

                //Sending ACKs
                mutex_get(client->rpc_handler->ack_mutex, WAIT_FOREVER);
                ack_t *entry,*tmp_entry;
	            list_for_each_entry_safe(entry, tmp_entry, &client->rpc_handler->ack_list, list, ack_t)
                {
                    _client_udp_ack(client, PKT_TYPE_RPC, entry->payload, entry->plen);
                    LOG_DEBUG("ack: %s", entry->payload);
                }
                ack_list_empty(&client->rpc_handler->ack_list);
                mutex_put(client->rpc_handler->ack_mutex);

                //Check MSG
                if(0 == _client_get_msg(client, &msg, 0)) {
                    if(CLIENT_MSG_EXIT == msg.type){
                        client->state = CLIENT_UDP_DEINIT;
                    }
                    else if(CLIENT_MSG_DEINIT == msg.type){
                        client->state = CLIENT_UDP_DEINIT;
                    }else{
                        LOG_WARN("Client msg[%d] droped!", msg.type);
                    }
                    _client_free_msg(&msg);
                }
                break;

            case CLIENT_UDP_DEINIT:
                LOG_INFO("Client State: DEINIT");         
                if(0 == _client_get_msg(client, &msg, WAIT_FOREVER)){
                    if(CLIENT_MSG_EXIT == msg.type){
                        socket_udp_close(&client->udp);
                        client->state = CLIENT_UDP_SHUTDOWN;
                        LOG_INFO("closed.");
                    }else{
                        LOG_WARN("ctrl msg[%d] droped!", msg.type);
                    }
                    _client_free_msg(&msg);
                }
                break;

            default:
                client->state = CLIENT_UDP_IDLE;
                break;
    	}

    }

    LOG_DEBUG("Exiting client_udp");

    client->state = CLIENT_UDP_SHUTDOWN;

    client->thread = NULL;
    thread_delete(NULL);

    return (function_cb_t)(0);
}

int client_udp_init(client_udp_t* client, pkt_encrypt_t encrypt_type)
{
    if(client != NULL) {
        memset(client, 0, sizeof(client_udp_t));
    }
    else
    {
        LOG_ERROR("Client UDP init failed!");
        return -1;
    }
    
    client->state = CLIENT_UDP_SHUTDOWN;

    //Set mac
    uint8_t mac[6] = {0};
    get_mac(mac);
    LOG_DEBUG("Get MAC address: "MACSTR"", MAC2STR(mac));

    //Set cipher ctx
    pkt_cipher_init(&client->cipher_ctx, encrypt_type);

    //Set local port
    client->local_port = DEFAULT_LOCAL_PORT;

    //init rpc_handler
    client->rpc_handler = malloc(sizeof(rpc_handler_t));
    if(NULL == client->rpc_handler) {
        LOG_ERROR("rpc_handler malloc failed!");
        goto error_exit;
    }

    if(0 != rpc_handler_init(client->rpc_handler, RPC_JSON)) {
        goto error_exit;
    }

    //Initialize control queue
    if(0 != queue_create(&client->msg_queue, DEFAULT_QUEUE_LEN, sizeof(client_udp_msg_t))) {
        LOG_ERROR("Client udp queue create failed!");
        goto error_exit;
    }

    client->state = CLIENT_UDP_IDLE;

    //Create thread
    if(0 != thread_create(&client->thread, "udp_client", client_udp_main, DEFAULT_UDP_CLIENT_TASK_STACK, (void*)client, PRIORITY_DEFAULT)) {
        client->state = CLIENT_UDP_SHUTDOWN;
        LOG_ERROR("Client udp thread create failed!");
        goto error_exit;
    }

    return 0;

error_exit:
    client_udp_deinit(client);
    rpc_handler_deinit(client->rpc_handler);
    return -1;
}

void client_udp_deinit(client_udp_t* client)
{
    if(client->msg_queue) {
        while(client->state != CLIENT_UDP_SHUTDOWN) {
            _client_set_msg(client, CLIENT_MSG_EXIT, NULL, 0, 100);
        }

        client_udp_msg_t msg = {
            .type = 0xff,
            .argv = NULL,
            .argc = 0
        };

        while(0 == _client_get_msg(client, &msg, 0)) {
            LOG_DEBUG("msg queue empty");
            _client_free_msg(&msg);
        }
        queue_delete(client->msg_queue);
        client->msg_queue = NULL;
    }

}
