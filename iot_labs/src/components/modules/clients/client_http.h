#ifndef __HTTPC_H__
#define __HTTPC_H__

#include "labs_platform.h"
#include "socket_tcp.h"
#include "fw_ota.h"

#define HTTP_TIMEOUT_MS       		(30*1000)
#define HTTP_CTRL_QUEUE_LEN         (4)
#define HTTP_TASK_SIZE              (4096)
#define HTTP_HEAD_LEN_MAX           (2048)

typedef enum{
    HTTP_REQ_GET = 0,
    HTTP_REQ_POST
}http_req_type_t;

typedef enum{
	CLIENT_HTTP_IDLE = 0,
	CLIENT_HTTP_CONNECTING,
	CLIENT_HTTP_REQ_HEAD_SENDING,
	CLIENT_HTTP_REQ_BODY_SENDING,
	CLIENT_HTTP_RESP_HEAD_RECVING,
	CLIENT_HTTP_RESP_BODY_RECVING,
	CLIENT_HTTP_DISCONNECTING,
	CLIENT_HTTP_DEAD
}http_state_t;

typedef struct {
    const char* url;
    const char* hostname;
    uint16_t port;
    http_req_type_t req_type;
	uint32_t timeout_ms;
    //const unsigned char* root_cert;
}http_req_params_t;

typedef struct {
    http_req_type_t req_type;
    const fw_ota_if_t* io_if;
    void *io_ctx;
    uint32_t timeout_ms;
    uint16_t port;
    char *hostname;
    char *url_domain;
    char *url_path;
    char url[0];
}http_params_t;

typedef struct {
    http_params_t *params;
	int file_len;
	int file_index;
	uint32_t timestamp;
	int content_len;
	char *head;
	int head_len;
	int head_index;
	uint8_t *body_pkt;
	int body_pkt_len;
	int body_processed;
	const char *err_msg;
	int buf_len;
	uint8_t *buf;
}http_ctx_t;

typedef struct httpc{
	socket_tcp_t tcp;
	http_state_t state;
	thread_handle_t task;
	queue_handle_t params_queue;
	http_ctx_t *ctx;
}http_t;
/**
 * @brief initialize an http client
 * @param client Pointer to a http handle
 * @return 0 on success -1 on failure
 */
int client_http_init(http_t *client);
/**
 * @brief Deinitialize an http client
 * @param client Pointer the the http client to deinitialize
 * @return 0 on success -1 on failure
 */
int client_http_deinit(http_t *client);
/**
 * @brief Send a http request
 * @param httpc Handle to http instance
 * @param req_params Pointer to the request parameters
 * @param io_if Pointer to the io interface
 * @param io_ctx Pointer to the io context
 * @return 0 on success -1 on failure
 */
int client_http_req(const http_t *httpc, http_req_params_t *req_params, const fw_ota_if_t* io_if, void* io_ctx);

#endif
