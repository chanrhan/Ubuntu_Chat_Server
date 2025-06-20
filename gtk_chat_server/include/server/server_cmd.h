#ifndef SERVER_CMD_H
#define SERVER_CMD_H

#include "server_sock.h"

typedef int (*cmd_func_t)(int fd, req_packet_t req);

#define DECLARE_CMDFUNC(str) int cmd_##str(int fd, req_packet_t req)

DECLARE_CMDFUNC(signup);
DECLARE_CMDFUNC(login);
DECLARE_CMDFUNC(sse_connect);

DECLARE_CMDFUNC(echo);
DECLARE_CMDFUNC(chat);
DECLARE_CMDFUNC(chat_react);


DECLARE_CMDFUNC(dm);
DECLARE_CMDFUNC(user_list);
DECLARE_CMDFUNC(start_private_chat);
DECLARE_CMDFUNC(accept_private_chat);
DECLARE_CMDFUNC(create_room);
DECLARE_CMDFUNC(enter_room);
DECLARE_CMDFUNC(exit_room);
DECLARE_CMDFUNC(room_list);

DECLARE_CMDFUNC(search_user_when_create_room);
DECLARE_CMDFUNC(update_chat_list);
DECLARE_CMDFUNC(delete_chat);

typedef struct cmd_func_pair_t{
    packet_type cmd;
    cmd_func_t cmd_func;
} cmd_func_pair_t;

extern cmd_func_pair_t cmd_func_list[];

extern int send_cmd_list_size;

int receive_cmd(int fd);

// int execute_cmd(int fd, req_packet_t req);

#endif