#ifndef CLIENT_RECV_CMD_H
#define CLIENT_RECV_CMD_H

// #include "common.h"
#include "client_sock.h"

typedef int (*recv_cmd_func_t)(res_packet_t* res_msg);

typedef struct recv_cmd_t{
    packet_type type;
    recv_cmd_func_t cmd_func;
}recv_cmd_t;

#define DECLARE_RECV_CMDFUNC(str) int recv_cmd_##str(res_packet_t* res)

DECLARE_RECV_CMDFUNC(signup);
DECLARE_RECV_CMDFUNC(login);
DECLARE_RECV_CMDFUNC(echo);
DECLARE_RECV_CMDFUNC(chat);
DECLARE_RECV_CMDFUNC(chat_updated);
DECLARE_RECV_CMDFUNC(dm);
DECLARE_RECV_CMDFUNC(user_list);
DECLARE_RECV_CMDFUNC(start_private_chat);
DECLARE_RECV_CMDFUNC(room_list);
DECLARE_RECV_CMDFUNC(exit);

// extern recv_cmd_t recv_cmd_list[];

// extern int recv_cmd_list_size;

void* sse_thread_func(void* arg);

int send_and_receive(packet_type wait_type, req_packet_t* req, res_packet_t* res);
int send_and_receive_sse(packet_type wait_type, req_packet_t* req, res_packet_t* res);

#endif