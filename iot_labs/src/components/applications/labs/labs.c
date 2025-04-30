#include "labs.h"

//test
void lab_0()
{
    uint8_t mac[6] = {0};
    get_mac(mac);

    //client
    client_udp_t *client_udp = malloc(sizeof(client_udp_t));
    if(NULL != client_udp) {
        memset(client_udp, 0, sizeof(client_udp_t));
    }
    else
    {
        LOG_ERROR("Client UDP malloc failed.");
    }
    
    client_udp_init(client_udp, ENCRYPT_TYPE_AES);
    client_udp_set_mode(client_udp, CLIENT_UDP_MODE_ONLINE);

    //protocol
    udp_packet_ctx_t ctx;

    char payload[32] = "This is a msg from your device.";
    uint8_t *packet = malloc(PKT_REAL_SIZE(strlen(payload)+1));
    if(NULL != packet) {
        memset(packet, 0, PKT_REAL_SIZE(strlen(payload)+1));
    }
    else
    {
        LOG_ERROR("packet buf malloc failed.");
    }

    pkt_wrapper_udp_init(&ctx, mac, 115554, PKT_TYPE_RPC, NULL, packet);
    pkt_wrapper_udp_update(&ctx, 32, payload);
    pkt_wrapper_udp_end(&ctx, &client_udp->cipher_ctx);
    //LOG_INFO("Wrapper output: %s", ctx.payload);

    pkt_parser_init(&ctx, packet, PKT_REAL_SIZE(strlen(payload)+1));
    pkt_parser_end(&ctx, &client_udp->cipher_ctx);
    LOG_INFO("Parser output: %s", ctx.payload);

    //json
    rpc_parse_test();
}

//jtag
void lab_1()
{
    //LED
    //get psk
}

//uart
void lab_2()
{

}

void lab_3()
{
    //secure key storage
}

//network security
void lab_4()
{

}

void lab_5()
{
    
}