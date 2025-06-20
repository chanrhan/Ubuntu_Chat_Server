#ifndef SERVER_SOCK_H
#define SERVER_SOCK_H

#include "socket_common.h"
#include <mysql/mysql.h>
#define SESSION_EXPIRED_TIME 60 // seconds


typedef struct session_t
{
    char* id;
    int fd;
    int sse_fd;
    struct session_t* next;
} session_t;

extern pthread_mutex_t mysql_mutex;

extern int s_sock;
extern int cli_sock;
extern char* buf;

extern int client_fds_num;

extern struct sockaddr_in ser_addr, cli_addr;
extern socklen_t client_len;

extern pthread_t client_thread;

extern struct pollfd client_sock_fds[10];

extern session_t* session_list;

int server_socket_init();

void* client_thread_func(void* arg);

char* add_session(int fd, char* id);

session_t* find_session_by_fd(int fd);
session_t* find_session_by_id(char* id);
int find_fd_by_id(char* id);
int find_sse_fd_by_id(char* id);

void print_session();

#endif