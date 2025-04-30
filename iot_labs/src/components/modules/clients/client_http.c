#include "client_http.h"

static int _client_http_idle(http_t *http)
{
	http_params_t *params = NULL;

	//LOG_DEBUG("idle");

	if(0 == queue_recv(http->params_queue, &params, WAIT_FOREVER)){
		if(params){
			http->ctx = calloc(1, U32_ALIGN4(sizeof(http_ctx_t))+U32_ALIGN4(HTTP_HEAD_LEN_MAX+1));
			if(NULL == http->ctx){
				goto error_exit;
			}
			http->ctx->buf_len = HTTP_HEAD_LEN_MAX;
			http->ctx->buf = ((uint8_t*)http->ctx) + U32_ALIGN4(sizeof(http_ctx_t));
			http->ctx->params = params;
			http->state = CLIENT_HTTP_CONNECTING;
		}
		else{
			http->state = CLIENT_HTTP_DEAD;
		}
	}
	else{
		http->state = CLIENT_HTTP_IDLE;
	}

	return 0;

error_exit:

	if(params){
		if(params->io_if)
			params->io_if->fp_error(params->io_ctx, (void*)"No Memory");

		free(params);
	}

	http->state = CLIENT_HTTP_IDLE;

	return -1;
}


static int _client_http_connecting(http_t *http)
{
	//LOG_DEBUG("connecting");

	net_addr_t addr;
	if(0 != plt_net_name2ip(http->ctx->params->url_domain, &addr)){
		http->ctx->err_msg = "DNS Failed";
		goto error_exit;
	}

    socket_tcp_config_t tcp_config = TCP_CONFIG_INIT_VALUE;
    tcp_config.ip_port.ip.s_addr = addr.s_addr;
    tcp_config.ip_port.port = http->ctx->params->port;
    if( 0 != socket_tcp_open(&http->tcp, &tcp_config) ){
        http->ctx->err_msg = "Open Failed";
        goto error_exit;
    }

	http->state = CLIENT_HTTP_REQ_HEAD_SENDING;

	return 0;

error_exit:

	LOG_ERROR("%s", http->ctx->err_msg);

	if(http->ctx->params->io_if)
		http->ctx->params->io_if->fp_error(http->ctx->params->io_ctx, (void*)http->ctx->err_msg);

	free(http->ctx->params);
	free(http->ctx);
	http->ctx = NULL;

	http->state = CLIENT_HTTP_IDLE;

	return -1;
}

static int _client_http_req_head_sending(http_t *http)
{
	http->ctx->head = (char*)http->ctx->buf;
	http->ctx->head_len = http->ctx->buf_len;
	http->ctx->head_index = 0;

	if(HTTP_REQ_GET == http->ctx->params->req_type){

		if(http->ctx->params->url_path)
			http->ctx->head_index += snprintf_safe(http->ctx->head+http->ctx->head_index, http->ctx->head_len-http->ctx->head_index, "GET %s HTTP/1.1\r\n", http->ctx->params->url_path);
		else
			http->ctx->head_index += snprintf_safe(http->ctx->head+http->ctx->head_index, http->ctx->head_len-http->ctx->head_index, "GET / HTTP/1.1\r\n");
		http->ctx->head_index += snprintf_safe(http->ctx->head+http->ctx->head_index, http->ctx->head_len-http->ctx->head_index, "Host: %s\r\n",http->ctx->params->hostname);
		http->ctx->head_index += snprintf_safe(http->ctx->head+http->ctx->head_index, http->ctx->head_len-http->ctx->head_index, "User-Agent: MIoT\r\n");
		http->ctx->head_index += snprintf_safe(http->ctx->head + http->ctx->head_index, http->ctx->head_len - http->ctx->head_index, "\r\n");

		//LOG_DEBUG("\r\n%s",http->ctx->head);
	}
	else if(HTTP_REQ_POST == http->ctx->params->req_type){

		http->ctx->file_len = http->ctx->params->io_if->fp_open(http->ctx->params->io_ctx, NULL);
		if(http->ctx->file_len < 0){
			http->ctx->err_msg = "IF Open Length Error";
			goto error_exit;
		}
		http->ctx->file_index = 0;
		http->ctx->content_len = http->ctx->file_len;

		if(http->ctx->params->url_path)
			http->ctx->head_index += snprintf_safe(http->ctx->head + http->ctx->head_index, http->ctx->head_len- http->ctx->head_index, "POST %s HTTP/1.1\r\n", http->ctx->params->url_path);
		else
			http->ctx->head_index += snprintf_safe(http->ctx->head + http->ctx->head_index, http->ctx->head_len- http->ctx->head_index, "POST / HTTP/1.1\r\n");
		http->ctx->head_index += snprintf_safe(http->ctx->head + http->ctx->head_index, http->ctx->head_len - http->ctx->head_index, "Host: %s\r\n",http->ctx->params->hostname);
		http->ctx->head_index += snprintf_safe(http->ctx->head + http->ctx->head_index, http->ctx->head_len - http->ctx->head_index, "User-Agent: MIoT\r\n");
		http->ctx->head_index += snprintf_safe(http->ctx->head + http->ctx->head_index, http->ctx->head_len - http->ctx->head_index, "Connection: keep-alive\r\n");
		http->ctx->head_index += snprintf_safe(http->ctx->head + http->ctx->head_index, http->ctx->head_len - http->ctx->head_index, "Content-Type: application/x-www-form-urlencoded\r\n");
		http->ctx->head_index += snprintf_safe(http->ctx->head + http->ctx->head_index, http->ctx->head_len - http->ctx->head_index, "Content-Length: %u\r\n", http->ctx->content_len);
		http->ctx->head_index += snprintf_safe(http->ctx->head + http->ctx->head_index, http->ctx->head_len - http->ctx->head_index, "\r\n");

		//LOG_DEBUG("\r\n%s", http->ctx->head);
	}
	else{
		http->ctx->err_msg = "No in&out Interface";
		goto error_exit;
	}

	int written_len = 0;

	{
		written_len = socket_tcp_write_to(&http->tcp, http->ctx->head, http->ctx->head_index);
	}

	if(http->ctx->head_index != written_len){
		http->ctx->err_msg = "TLS/TCP Write Error";
		goto error_exit;
	}

	if(HTTP_REQ_GET == http->ctx->params->req_type){
		http->ctx->head = (char*)http->ctx->buf;
		http->ctx->head_len = http->ctx->buf_len;
		http->ctx->head_index = 0;
		http->ctx->timestamp = ms_now();
		http->state = CLIENT_HTTP_RESP_HEAD_RECVING;
	}
	else if(HTTP_REQ_POST == http->ctx->params->req_type){
		http->ctx->body_pkt = http->ctx->buf;
		http->ctx->body_pkt_len = http->ctx->buf_len;
		http->ctx->body_processed = 0;
		http->state = CLIENT_HTTP_REQ_BODY_SENDING;
	}

	return 0;

error_exit:

	LOG_ERROR("%s", http->ctx->err_msg);

	http->ctx->content_len = 0;
	http->ctx->body_processed = -1;

	http->state = CLIENT_HTTP_DISCONNECTING;

	return -1;
}

static int _client_http_req_body_sending(http_t *http)
{
	if(HTTP_REQ_POST == http->ctx->params->req_type){
		while(http->ctx->body_processed < http->ctx->content_len){

			int pkt_len = MIN(http->ctx->content_len-http->ctx->body_processed, http->ctx->body_pkt_len);

			pkt_len = http->ctx->params->io_if->fp_read(http->ctx->params->io_ctx, http->ctx->body_pkt, pkt_len);

			if(pkt_len <= 0){
				http->ctx->err_msg = "IF Read Error";
				goto error_exit;
			}

			int written_len = 0;

			{
				written_len = socket_tcp_write_to(&http->tcp, http->ctx->body_pkt, pkt_len);
			}

			if(pkt_len == written_len)
				http->ctx->body_processed += pkt_len;
			else{
				http->ctx->err_msg = "TLS/TCP Write Error";
				goto error_exit;
			}

			LOG_INFO("<== %d%%",(http->ctx->body_processed * 100 / http->ctx->content_len));
		}

		http->ctx->head = (char*)http->ctx->buf;
		http->ctx->head_len = http->ctx->buf_len;
		http->ctx->head_index = 0;
		http->ctx->timestamp = ms_now();

		http->state = CLIENT_HTTP_RESP_HEAD_RECVING;
	}
	else{
		http->ctx->err_msg = "Unknow Error";
		goto error_exit;
	}

	return 0;

error_exit:

	LOG_ERROR("%s", http->ctx->err_msg);

	http->ctx->content_len = 0;
	http->ctx->body_processed = -1;

	http->state = CLIENT_HTTP_DISCONNECTING;

	return -1;
}

static int _client_http_resp_head_recving(http_t *http)
{
	if(http->ctx->head_index >= http->ctx->head_len){
		http->ctx->err_msg = "HTTP Resp too long";
		goto error_exit;
	}

	char* pkt_ptr = http->ctx->head + http->ctx->head_index;
	int pkt_len = http->ctx->head_len - http->ctx->head_index;

	{
		pkt_len = socket_tcp_read_from(&http->tcp, pkt_ptr, pkt_len);
	}

	if(pkt_len < 0){
		http->ctx->err_msg = "TLS/TCP Read Error";
		goto error_exit;
	}
	else if(pkt_len == 0){
		goto try_again;
	}
	http->ctx->timestamp = ms_now();
	http->ctx->head_index += pkt_len;
	http->ctx->head[http->ctx->head_index] = '\0';

	if(HTTP_REQ_GET == http->ctx->params->req_type){
        if(http->ctx->head_index < sizeof("HTTP/1.1 200 OK")-1){
            goto try_again;
        }

        if( 0 != memcmp(http->ctx->head, "HTTP/1.1 200 OK", sizeof("HTTP/1.1 200 OK")-1) && 0 != memcmp(http->ctx->head, "HTTP/1.0 200 OK", sizeof("HTTP/1.0 200 OK")-1)){
            http->ctx->err_msg = http->ctx->head;
            goto error_exit;
        }

		char *end = strstr(http->ctx->head, "\r\n\r\n");
		if(NULL == end){
			goto try_again;
		}
		*end = '\0';
		end += sizeof("\r\n\r\n") - 1;

		////LOG_DEBUG("\r\n%s\r\n", http->ctx->head);

		char *c = strstr(http->ctx->head, "Content-Length:");
		if(NULL == c){
			http->ctx->err_msg = "GET Content Length Error";
			goto error_exit;
		}
		http->ctx->content_len = atoi(c + sizeof("Content-Length:") - 1);
		if(http->ctx->content_len <= 0){
			http->ctx->err_msg = "GET Content-Length Error";
			goto error_exit;
		}

		LOG_INFO("Content-Length %d", http->ctx->content_len);

        http->ctx->file_len = http->ctx->content_len;
        http->ctx->file_index = 0;

        if(0 != http->ctx->params->io_if->fp_open(http->ctx->params->io_ctx, (void*)&http->ctx->file_len)){
            http->ctx->err_msg = "IF Open Error";
            goto error_exit;
        }

		http->ctx->body_pkt = (uint8_t*)end;
		http->ctx->body_pkt_len = (uint8_t*)(http->ctx->head + http->ctx->head_index) - http->ctx->body_pkt;
		http->ctx->body_processed = 0;

		http->state = CLIENT_HTTP_RESP_BODY_RECVING;
	}else if(HTTP_REQ_POST == http->ctx->params->req_type){

		if(http->ctx->head_index < sizeof("HTTP/1.1 200 OK")-1){
			goto try_again;
		}

		if( 0 != memcmp(http->ctx->head, "HTTP/1.1 200 OK", sizeof("HTTP/1.1 200 OK")-1) ){
			http->ctx->err_msg = http->ctx->head;
			goto error_exit;
		}

		LOG_INFO("POST Done(%ubytes).", http->ctx->body_processed );

		http->state = CLIENT_HTTP_DISCONNECTING;
	}
	else{
		http->ctx->err_msg = "Unknow Error";
		goto error_exit;
	}

	return 0;

try_again:

	if(ms_elapsed(http->ctx->timestamp) > http->ctx->params->timeout_ms){
		http->ctx->err_msg = "Time Out";
		goto error_exit;
	}

	http->state = CLIENT_HTTP_RESP_HEAD_RECVING;

	return 0;

error_exit:

	LOG_ERROR("%s", http->ctx->err_msg);

	http->ctx->content_len = 0;
	http->ctx->body_processed = -1;

	http->state = CLIENT_HTTP_DISCONNECTING;

	return -1;
}

static int _client_http_resp_body_recving(http_t *http)
{
	if(HTTP_REQ_GET == http->ctx->params->req_type){

		if(http->ctx->body_pkt_len == 0){

			http->ctx->body_pkt = http->ctx->buf;
			http->ctx->body_pkt_len = http->ctx->buf_len;

			{
				http->ctx->body_pkt_len = socket_tcp_read_from(&http->tcp, http->ctx->body_pkt, http->ctx->body_pkt_len);
			}
		}

		if(http->ctx->body_pkt_len < 0){
			http->ctx->err_msg = "TLS/TCP Read Error";
			goto error_exit;
		}
		else if(http->ctx->body_pkt_len == 0){
			goto try_again;
		}
		http->ctx->timestamp = ms_now();

		int last_len = MIN(http->ctx->body_pkt_len, http->ctx->content_len - http->ctx->body_processed);

		if(last_len){
			if((last_len != http->ctx->params->io_if->fp_write(http->ctx->params->io_ctx, http->ctx->body_pkt, last_len))){
				http->ctx->err_msg = "IF Write Error";
				goto error_exit;
			}
		}

		http->ctx->body_processed += last_len;
		http->ctx->body_pkt_len = 0;

		LOG_INFO("==> %d%%",(http->ctx->body_processed * 100 / http->ctx->content_len));
        ms_sleep(10);
		if(http->ctx->body_processed >= http->ctx->content_len){
			LOG_INFO("GET Done(%ubytes).", http->ctx->body_processed );
			http->state = CLIENT_HTTP_DISCONNECTING;
		}
		else{
			http->state = CLIENT_HTTP_RESP_BODY_RECVING;
		}
	}
	else{
		http->ctx->err_msg = "Unknow Error";
		goto error_exit;
	}

	return 0;

try_again:

	if(ms_elapsed(http->ctx->timestamp) > http->ctx->params->timeout_ms){
		http->ctx->err_msg = "Time Out";
		goto error_exit;
	}

	http->state =  CLIENT_HTTP_RESP_BODY_RECVING;

	return 0;

error_exit:

	http->state =  CLIENT_HTTP_DISCONNECTING;

	LOG_ERROR("%s", http->ctx->err_msg);

	return -1;
}

static int _client_http_disconnecting(http_t *http)
{
	if(http->ctx->body_processed < http->ctx->content_len){
		goto error_exit;
	}

	http->ctx->file_index += http->ctx->body_processed;
	if(http->ctx->file_index >= http->ctx->file_len){
		goto file_finished;
	}

error_exit:

	LOG_ERROR("disconnecting");

	{
		socket_tcp_close(&http->tcp);
	}

	if(http->ctx->params->io_if){
		http->ctx->params->io_if->fp_error(http->ctx->params->io_ctx, (void*)http->ctx->err_msg);
	}

	free(http->ctx->params);
	free(http->ctx);
	http->ctx = NULL;

	http->state = CLIENT_HTTP_IDLE;

	return -1;

file_finished:
	LOG_INFO("File Done(%ubytes).", http->ctx->file_index);

	{
		socket_tcp_close(&http->tcp);
	}

	if(http->ctx->params->io_if){
		if(0 != http->ctx->params->io_if->fp_close(http->ctx->params->io_ctx)){
			http->ctx->err_msg = "IF Close Err";
			if(http->ctx->params->io_if){
				http->ctx->params->io_if->fp_error(http->ctx->params->io_ctx, (void*)http->ctx->err_msg);
			}
		}
	}

	free(http->ctx->params);
	free(http->ctx);
	http->ctx = NULL;

	http->state = CLIENT_HTTP_IDLE;

	return 0;
}

static function_cb_t client_http_task(void * arg)
{
	http_t *http = (http_t*)arg;
    LOG_DEBUG("Client http thread start");
    while(http->state != CLIENT_HTTP_DEAD) {
    	switch(http->state){
    	case CLIENT_HTTP_IDLE:
    		_client_http_idle(http);
    		break;
    	case CLIENT_HTTP_CONNECTING:
    		_client_http_connecting(http);
    		break;
    	case CLIENT_HTTP_REQ_HEAD_SENDING:
    		_client_http_req_head_sending(http);
    		break;
    	case CLIENT_HTTP_REQ_BODY_SENDING:
    		_client_http_req_body_sending(http);
    		break;
    	case CLIENT_HTTP_RESP_HEAD_RECVING:
    		_client_http_resp_head_recving(http);
    		break;
    	case CLIENT_HTTP_RESP_BODY_RECVING:
    		_client_http_resp_body_recving(http);
    		break;
    	case CLIENT_HTTP_DISCONNECTING:
    		_client_http_disconnecting(http);
    		break;
    	default:
    		http->state = CLIENT_HTTP_IDLE;
    		break;
    	}
    }

    LOG_DEBUG("safe exit...");

    thread_delete(NULL);

    return (function_cb_t)(0);
}

int client_http_deinit(http_t *http)
{
	http_params_t *params = NULL;

	if(http->params_queue){
		while(http->state != CLIENT_HTTP_DEAD){
			params = NULL;
			queue_send(http->params_queue, &params, 100);
			//LOG_DEBUG("destroy exit waiting");
		}
		while(0 == queue_recv(http->params_queue, &params, 0)){
			if(params){
				if(params->io_if)
					params->io_if->fp_error(params->io_ctx, (void*)"HTTC DEAD");

				free(params);
			}
		}

		queue_delete(http->params_queue);
		http->params_queue = NULL;
	}

    if(http) {
        free(http);
    }
    LOG_WARN("client http deinit.");
	return 0;
}


int client_http_init(http_t *http)
{
    LOG_DEBUG("Client http init start");

    if (http != NULL) {
	    memset(http, 0, sizeof(http_t));
    }
    else {
        LOG_ERROR("http is null!");
        goto err_exit;
    }

	http->state = CLIENT_HTTP_DEAD;

	if(0 != queue_create(&http->params_queue, HTTP_CTRL_QUEUE_LEN, sizeof(http_params_t *))){
		LOG_ERROR("params_queue create failed");
		goto err_exit;
	}

	http->state = CLIENT_HTTP_IDLE;
	if(0 != thread_create(&http->task, "client_http", client_http_task, HTTP_TASK_SIZE, (void*)http, PRIORITY_DEFAULT)){
		http->state = CLIENT_HTTP_DEAD;
		LOG_ERROR("task create failed");
		goto err_exit;
	}

	return 0;

err_exit:

	client_http_deinit(http);

	return -1;
}

int client_http_req(const http_t *http, http_req_params_t *req_params, const fw_ota_if_t* io_if, void* io_ctx)
{
	int ret = 0;

	uint16_t port ;
	const char *domain;
	size_t domain_len;
	const char *path;
	size_t path_len;
	const char *hostname;
	size_t hostname_len;

	if(NULL == http || CLIENT_HTTP_DEAD == http->state || NULL == http->params_queue){
		LOG_ERROR("http is dead.");
		return -1;
	}

	if(NULL == req_params){
		LOG_ERROR("req_params is null.");
		return -1;
	}

	hostname = req_params->hostname;
	if(hostname){
		hostname_len = strnlen(hostname, PLT_URL_SIZE_MAX);
		if(hostname_len >= PLT_URL_SIZE_MAX){
			LOG_ERROR("hostname too long");
			return -1;
		}
	}
	else{
		hostname_len = 0;
	}

	size_t url_len = strnlen(req_params->url, PLT_URL_SIZE_MAX);
	if(url_len >= PLT_URL_SIZE_MAX || url_len < sizeof("http://x.x")){
		LOG_ERROR("url too long or too short.");
		return -1;
	}

	if(NULL == io_if){
		LOG_ERROR("no io interface.");
		return -1;
	}

	if(HTTP_REQ_GET != req_params->req_type && HTTP_REQ_POST != req_params->req_type){
		LOG_ERROR("req type not supported.");
		return -1;
	}

	if(0 == strncmp(req_params->url, "http://", sizeof("http://") - 1))
	{

		if(req_params->port){
			port = req_params->port;
		}
		else{
			port = 80;
		}
		domain = req_params->url + sizeof("http://") - 1;
	}
	else
	{
		LOG_ERROR("url err(%s).", req_params->url);
		return -1;
	}

	path = strchr(domain,'/');
	if(path){
		domain_len = path - domain;
		path_len = strlen(path);
	}
	else{
		domain_len = strlen(domain);
		path = "";
		path_len = 0;
	}

	if(0 == hostname_len){
		hostname = domain;
		hostname_len = domain_len;
	}
    LOG_DEBUG("domain len = %d, path len = %d, hostname len = %d", domain_len, path_len, hostname_len);
    LOG_DEBUG("malloc size = %d", sizeof(http_params_t)+domain_len+1+path_len+1+hostname_len+1);

    http_params_t *params = (http_params_t *)malloc(sizeof(http_params_t)+domain_len+1+path_len+1+hostname_len+1);
	
    if(NULL == params){
		LOG_ERROR("http params malloc error.");
		return -1;
	}

	params->req_type = req_params->req_type;
	params->io_if = io_if;
	params->io_ctx = io_ctx;

	if(0 == req_params->timeout_ms){
		params->timeout_ms = HTTP_TIMEOUT_MS;
	}
	else{
		params->timeout_ms = MIN(req_params->timeout_ms, HTTP_TIMEOUT_MS);
	}

	params->port = port;
    LOG_DEBUG("client http: set port = %d", params->port);

	params->url_domain = params->url;
	params->url_path = params->url + domain_len + 1;
	params->hostname = params->url + domain_len + 1 + path_len + 1;
	strncpy(params->url_domain, domain, domain_len);
	params->url_domain[domain_len] = '\0';
	strncpy(params->url_path, path, path_len);
	params->url_path[path_len] = '\0';
	strncpy(params->hostname, hostname, hostname_len);
	params->hostname[hostname_len] = '\0';

	LOG_DEBUG("url = %s", req_params->url);
	LOG_DEBUG("domain = %s::%u", params->url_domain, params->port);
	LOG_DEBUG("path = %s", params->url_path);
	LOG_DEBUG("hostname = %s", params->hostname);

	ret = queue_send(http->params_queue, &params, 0);
	if(0 != ret){
		LOG_ERROR("Queue is messed up.");
		free(params);
	}

	return ret;
}

