#ifndef CLIENT_SOCK_H
#define CLIENT_SOCK_H

#include "socket_common.h"

extern int sock_fd;
extern int sse_fd;

typedef struct client_session{
    char* id;
    char* access_token;
    char* refresh_token;
    int expried_time;
    int curr_room_id;
}client_session;

extern pthread_t sse_thread;
extern pthread_t gtk_thread;


extern client_session cli_session;

#endif