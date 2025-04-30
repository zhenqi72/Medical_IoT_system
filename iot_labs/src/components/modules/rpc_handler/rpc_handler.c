#include "rpc_handler.h"
#include "debug_log.h"
#include "jsmn.h" //https://zserge.com/jsmn/

/**********************************************************/
static int _ack_list_insert(list_head_t *head, char* payload, size_t plen);
static void _rpc_get_data(rpc_handler_t* rpc_handler, int argc, char **argv);
static void _rpc_fw_upgrade(rpc_handler_t* rpc_handler, int argc, char **argv);

static rpc_entry rpc_list[] = \
{
    { "GET_DATA",     _rpc_get_data     },
    { "UPGRADE",      _rpc_fw_upgrade   },
    //add more
};

/**********************************************************/
static void _rpc_get_data(rpc_handler_t* rpc_handler, int argc, char **argv)
{
    //_ack_list_insert
}

void test_rpc_fw_upgrade(rpc_handler_t* rpc_handler, int argc, char *url)
{
    //ota
	http_req_params_t req_params = {
		.url = url,
		.hostname = NULL,
		.port = 0,
		.req_type = HTTP_REQ_GET,
		.timeout_ms = 0
	};
    LOG_DEBUG("OTA TEST BEGIN!!!!!!!!");
    LOG_DEBUG("Free heap size: %u bytes",esp_get_free_heap_size());
	client_http_req(rpc_handler->http, &req_params, &rpc_handler->fw_ota->ota_if, rpc_handler->fw_ota);

}

static void _rpc_fw_upgrade(rpc_handler_t* rpc_handler, int argc, char **argv)
{
    //ota
    mutex_get(rpc_handler->ack_mutex, WAIT_FOREVER);
    _ack_list_insert(&rpc_handler->ack_list, RPC_ACK_OK_STR, strlen(RPC_ACK_OK_STR)+1);
    mutex_put(rpc_handler->ack_mutex);
       
	http_req_params_t req_params = {
		.url = argv[0],
		.hostname = NULL,
		.port = 0,
		.req_type = HTTP_REQ_GET,
		.timeout_ms = 0
	};

	client_http_req(rpc_handler->http, &req_params, &rpc_handler->fw_ota->ota_if, rpc_handler->fw_ota);
}

static int _ack_list_insert(list_head_t *head, char* payload, size_t plen)
{
	ack_t *entry = calloc(1, sizeof(ack_t));

	if(NULL == entry){
		return -1;
	}

	INIT_LIST_HEAD(&entry->list);

	entry->plen = plen;

	memcpy(&entry->payload, payload, plen);

	list_add_tail(&entry->list, head);

	return 0;
}

int ack_list_empty(list_head_t *head)
{
	ack_t *entry,*tmp_entry;
	list_for_each_entry_safe(entry, tmp_entry, head, list, ack_t)
	{
		list_del(&entry->list);
		free(entry);
	}

	return 0;
}

//Premature. Can't parse complex JSON yet.
static int _command_parse(void *payload, jsmntok_t *tokens, unsigned int token_num, rpc_t *ctx)
{
    int ret = 0;
    int len = 0;
    bool got_cmd = false;
    bool got_param = false;

    for(int i = 0; i < token_num && (!got_cmd || !got_param); i++) {
        //get length
        len = tokens[i].end - tokens[i].start;

        if(0 == memcmp(payload + tokens[i].start, "COMMAND", MAX(strlen("COMMAND"), (size_t)len))) {
            len = tokens[i+1].end - tokens[i+1].start;
            //Check length
            if(RPC_COMMAND_LEN_MAX <= len) {
                LOG_ERROR("Invalid COMMAND len!");
                ret = -1;
                break;
            }

            //Extract COMMAND name
            memcpy(ctx->command, payload+tokens[i+1].start, (size_t)len);

            //Add '\0'
            ctx->command[len] = '\0';
            LOG_DEBUG("COMMAND \'%s\' found", ctx->command);
            got_cmd = true;
        }
        else if(0 == memcmp(payload + tokens[i].start, "PARAMS", MAX(strlen("PARAMS"), (size_t)len))) {
            len = tokens[i+1].end - tokens[i+1].start;
            //Check length
            if(RPC_PARAMS_LEN_MAX <= len) {
                LOG_ERROR("Invalid PARAMS len!");
                ret = -1;
                break;
            }

            //Extract params
            memcpy(ctx->params, payload+tokens[i+1].start, (size_t)len);

            //Add '\0'
            ctx->params[len] = '\0';
            LOG_DEBUG("PARAMS: %s", ctx->params);
            got_param = true;
        }
    }

    if(!got_cmd || !got_param) {
        ret = -1;
    }

    return ret;
}

static void _rpc_handler_free(rpc_t *rpc)
{
    if(rpc) {
        free(rpc);
        rpc = NULL;
    }
}

static void _command_execute(rpc_handler_t *rpc_handler, rpc_t rpc)
{
    //search COMMAND
    LOG_DEBUG("Searching COMMAND list...");

    int i = 0;

    if((!rpc.command) || (strlen(rpc.command) == 0)) {
        LOG_ERROR("COMMAND is NULL!");
        return;
    }

    while (i < RPC_NUM_MAX) {
        if (rpc_list[i].name != NULL && strcmp(rpc_list[i].name, rpc.command) == 0) {
            //enter
            LOG_DEBUG("COMMAND %s found.", rpc.command);

            rpc_list[i].function(rpc_handler, 1, &rpc.params);//fixme: should consider multi params
            _rpc_handler_free(&rpc);
            return;
        }
        i++;
    }
    LOG_ERROR("COMMAND not found!");

    return;
}

static int _rpc_handler_get(rpc_handler_t *rpc_handler, rpc_t *rpc)
{
    rpc->command = NULL;
    rpc->params = NULL;

    mutex_get(rpc_handler->rpc_mutex, WAIT_MS2TICK(50));

    if(0 == queue_recv(rpc_handler->rpc_queue, rpc, WAIT_MS2TICK(50))) {
        LOG_DEBUG("rpc handler: content get.");
        return 0;
    }

    mutex_put(rpc_handler->rpc_mutex);

    return -1;
}

static function_cb_t rpc_handler_main(void * arg)
{
    rpc_handler_t *rpc_handler = (rpc_handler_t*)arg;

    rpc_t rpc = {
        .command = NULL,
        .params = NULL
    };

    while(rpc_handler->active) {
        if(0 == _rpc_handler_get(rpc_handler, &rpc)) {
            _command_execute(rpc_handler, rpc);
        }
        else {
            rpc.command = NULL;
            rpc.params = NULL;
        }
    }

    LOG_DEBUG("Ending rpc_handler...");

    rpc_handler->thread = NULL;
    thread_delete(NULL);//self delete
    rpc_handler->active = false;

    return (function_cb_t)(0);
}

int rpc_handler_init(rpc_handler_t *rpc_handler, content_type_t type)
{
    if(rpc_handler != NULL) {
        memset(rpc_handler, 0, sizeof(rpc_handler_t));
    }
    else {
         LOG_ERROR("rpc_handler init failed!");
         goto error_exit;
    }

    //active
    rpc_handler->active = true;

    //content type
    rpc_handler->type = type;

    //rpc queue
    if(0 != queue_create(&rpc_handler->rpc_queue, DEFAULT_RPC_QUEUE_LEN, sizeof(rpc_t))) {
        LOG_ERROR("rpc_handler queue create failed!");
        goto error_exit;
    }

    //rpc mutex
    if(0 != mutex_create(&rpc_handler->rpc_mutex)) {
        LOG_ERROR("rpc_handler rpc mutex create failed!");
        goto error_exit;
    }

    //ack list
    INIT_LIST_HEAD(&rpc_handler->ack_list);

    //ack mutex
    if(0 != mutex_create(&rpc_handler->ack_mutex)) {
        LOG_ERROR("rpc_handler ack mutex create failed!");
        goto error_exit;
    }

    //http client
    rpc_handler->http = malloc(sizeof(http_t));
    if (rpc_handler->http == NULL) {
	    goto error_exit;
    }

    if(0 != client_http_init(rpc_handler->http)) {
        LOG_ERROR("rpc_handler http init failed!");
        goto error_exit;
    }

    //fw_ota
    rpc_handler->fw_ota = malloc(sizeof(fw_ota_t));
    if (rpc_handler->fw_ota == NULL) {
	    goto error_exit;
    }

    if(0 != fw_ota_init(rpc_handler->fw_ota)) {
        LOG_ERROR("rpc_handler http init failed!");
        goto error_exit;
    }

    //thread
    if(0 != thread_create(&rpc_handler->thread, "rpc_handler", rpc_handler_main, DEFAULT_RPC_HANDLER_TASK_STACK, (void*)rpc_handler, PRIORITY_DEFAULT)) {
        
        LOG_ERROR("rpc_handler thread create failed!");
        goto error_exit;
    }

    return 0;

error_exit:
    rpc_handler_deinit(rpc_handler);
    return -1;
}

void rpc_handler_deinit(rpc_handler_t *rpc_handler)
{
    LOG_DEBUG("rpc_handler deinit...");
    //active
    rpc_handler->active = false;

    //type
    rpc_handler->type = RPC_INVALID;

    //queue
    if(rpc_handler->rpc_queue) {
        mutex_get(rpc_handler->rpc_mutex, WAIT_FOREVER);
        queue_delete(rpc_handler->rpc_queue);
        mutex_put(rpc_handler->rpc_mutex);
        rpc_handler->rpc_queue = NULL;
    }

    //deinit executers

    //http
    client_http_deinit(rpc_handler->http);

    //fw_ota
    fw_ota_deinit(rpc_handler->fw_ota);

    mutex_delete(rpc_handler->rpc_mutex);
    rpc_handler->rpc_mutex = NULL;

    mutex_delete(rpc_handler->ack_mutex);
    rpc_handler->ack_mutex = NULL;
}

//Currently only support single-task implementations.
int rpc_handler_put(rpc_handler_t *rpc_handler, void *payload, size_t plen, content_type_t type)
{
    if (type != rpc_handler->type) {
        LOG_ERROR("Invalid content type.");
        goto err_exit;
    }
    
    jsmn_parser parser = {0};
    jsmntok_t tokens[RPC_TOKEN_MAX] = {0};

    jsmn_init(&parser);

    rpc_t rpc = {
        .command = NULL,
        .params = NULL
    };

    rpc.command = malloc(RPC_COMMAND_LEN_MAX + RPC_PARAMS_LEN_MAX);
    if(NULL == rpc.command) {
        LOG_ERROR("rpc malloc failed!");
        goto err_exit;
    }
    rpc.params = rpc.command + RPC_COMMAND_LEN_MAX;

    int ret = 0;
    ret = jsmn_parse(&parser, (const char *)payload, plen, tokens, RPC_TOKEN_MAX);
    if(0 > ret) {
        LOG_ERROR("JSON parse failed: ret = %d", ret);
        goto err_exit;
    }
    
    ret = _command_parse(payload, tokens, ret, &rpc);
    if(ret)
    {
        if(0 != mutex_get(rpc_handler->rpc_mutex, WAIT_MS2TICK(50))) {//WAIT_TIME: TBD
            LOG_ERROR("rpc_queue not available: put");
        }

        if (0 > queue_send(&rpc_handler->rpc_queue, &rpc, WAIT_MS2TICK(50))) {
            LOG_ERROR("rpc_queue send failed! maybe is full");
        }

        mutex_put(rpc_handler->rpc_mutex);
    }

    return ret;

err_exit:
    if(rpc.command) {
        free(rpc.command);
    }

    return -1;
}

void rpc_parse_test()
{
    LOG_DEBUG("Starting rpc_parse_test...");
    char json_example[55] = "{\"COMMAND\":\"UPGRADE\",\"PARAMS\":\"http://thisisatesturl\"}";

    jsmn_parser parser = {0};
    jsmntok_t tokens[RPC_TOKEN_MAX] = {0};

    jsmn_init(&parser);

    rpc_t rpc = {
        .command = NULL,
        .params = NULL
    };

    rpc.command = malloc(RPC_COMMAND_LEN_MAX + RPC_PARAMS_LEN_MAX);
    if(NULL == rpc.command) {
        LOG_ERROR("rpc malloc failed!");
        return;
    }
    rpc.params = rpc.command + RPC_COMMAND_LEN_MAX;

    int ret = 0;
    ret = jsmn_parse(&parser, (const char *)json_example, strlen(json_example)+1, tokens, RPC_TOKEN_MAX);
    if(0 > ret) {
        LOG_ERROR("JSON parse failed: ret = %d", ret);
        return;
    }

    LOG_DEBUG("token amount: %d", ret);
    for(int i=0; i<ret; i++) {
        LOG_DEBUG("token start: %d", tokens[i].start);
        LOG_DEBUG("token end: %d", tokens[i].end);
        LOG_DEBUG("token size: %d", tokens[i].size);
    }

    
    ret = _command_parse(json_example, tokens, ret, &rpc);
    LOG_DEBUG("rpc command = %s", rpc.command);
    LOG_DEBUG("rpc params = %s", rpc.params);

    LOG_DEBUG("rpc_parse_test finished.");

}