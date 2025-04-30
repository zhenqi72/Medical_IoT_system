#ifndef __RPC_HANDLER_H__
#define __RPC_HANDLER_H__

#include "os_func.h"
#include "net.h"
#include "list/list.h"
#include "client_http.h"
#include "fw_ota.h"

#define RPC_TOKEN_MAX                       10
#define DEFAULT_RPC_HANDLER_TASK_STACK      4096
#define DEFAULT_RPC_QUEUE_LEN               8

//{"COMMAND":"UPGRADE", "PARAMS":"http://xxxxxxxx.xxx"}
#define RPC_COMMAND_LEN_MAX                 16
#define RPC_PARAMS_LEN_MAX                  512

#define RPC_NUM_MAX                         10

#define RPC_ACK_OK_STR                      "{\"RESULT\":\"OK\"}"

typedef enum{
    RPC_INVALID = 0,
    RPC_JSON,
    RPC_RAW
}content_type_t;

typedef struct rpc_handler{
    bool active;
    content_type_t type;
    thread_handle_t thread;

    queue_handle_t rpc_queue;
    mutex_handle_t rpc_mutex;
    
    list_head_t ack_list;
    mutex_handle_t ack_mutex;

    //executers
    http_t *http;
    fw_ota_t *fw_ota;
}rpc_handler_t;

typedef struct{
    char *command;
    char *params;
}rpc_t;

typedef struct{
    const char *name;
    void (*function)(rpc_handler_t* rpc_handler, int argc, char **argv);
}rpc_entry;

typedef struct{
    list_head_t list;
    char *payload;
    size_t plen;
}ack_t;

void rpc_parse_test();
int rpc_handler_init(rpc_handler_t *rpc_handler, content_type_t type);
int rpc_handler_put(rpc_handler_t *rpc_handler, void *payload, size_t plen, content_type_t type);
void rpc_handler_deinit(rpc_handler_t *rpc_handler);
int ack_list_empty(list_head_t *head);//fixme: empty outside?

void test_rpc_fw_upgrade(rpc_handler_t* rpc_handler, int argc, char *url);

#endif
