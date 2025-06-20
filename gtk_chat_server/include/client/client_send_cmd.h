#ifndef CLIENT_SEND_CMD_H
#define CLIENT_SEND_CMD_H

#include "common.h"

// typedef int (*send_cmd_func_t)(res_packet_t* res_msg);


// typedef struct send_cmd_t{
//     packet_type cmd;
//     send_cmd_func_t cmd_func;
// }send_cmd_t;


#define DECLARE_SEND_CMDFUNC(str) int send_cmd_##str(req_packet_t* req)

DECLARE_SEND_CMDFUNC(signup);
DECLARE_SEND_CMDFUNC(login);
DECLARE_SEND_CMDFUNC(echo);
DECLARE_SEND_CMDFUNC(chat);
DECLARE_SEND_CMDFUNC(dm);
DECLARE_SEND_CMDFUNC(user_list);
DECLARE_SEND_CMDFUNC(start_private_chat);
DECLARE_SEND_CMDFUNC(room_list);

// extern send_cmd_t req_cmd_lsit[];

// extern int req_cmd_lsit_size;

#endif