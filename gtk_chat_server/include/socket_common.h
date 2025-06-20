#ifndef SOCKET_COMMON_H
#define SOCKET_COMMON_H

#include "jwt_common.h"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>

typedef enum chat_type{
    CHAT = 0,
    LONG_CHAT,
    EMOJI,
    IMAGE,
}chat_type;

typedef enum chat_react_type{
    LOVE = 0,
    LIKE,
    CHECK,
    LAUGH,
    SURPRISE,
    SAD
}chat_react_type;

typedef struct chat_react_vo{
    chat_react_type react_type;
    char owner_id[16];
    // char owner_name[16];
}chat_react_vo;

typedef struct user_vo{
    char id[16];
    char name[16];
    char pwd[128];
}user_vo;

typedef struct room_vo{
    int room_id;
    int member_num;
    int updated_chat_count;
    char room_name[16];
    char last_chat[64];
    char last_chat_time[32];
}room_vo;

typedef struct chat_vo{
    int chat_id;
    int reply_chat_id;
    int non_read_count;
    int emo_list[6];
    int emo_total_cnt;
    int deleted;
    chat_type chat_type;
    char owner_id[16];
    char owner_name[16];
    char chat_time[32];
    char text[64];
    // chat_react_vo reacts[16];
}chat_vo;

typedef struct mysql_bind_vo{
    int data_len;
    char data[2048];
}mysql_bind_vo;

// int send_request(int sock, req_packet_t* req);

// int send_response(int sock, res_packet_t* res);

#endif
