#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <dirent.h>

#define MAX_PACKET_SIZE 16384

#define SUCCESS 1
#define FAIL -1

// SOCKET
#define PORT 9000
#define BUF_SIZE 1024
#define IP_ADDR "127.0.0.1"
#define MAX_CLIENT 20

// MYSQL
#define MYSQL_HOSTNAME "localhost"
#define MYSQL_USERNAME "chan"
#define MYSQL_PWD "091504"
#define MYSQL_DATABASE "talks"

typedef enum packet_type{
    SIGNUP = 10,
    LOGIN,
    SSE_CONNECT,
    ECHO,
    SEND_CHAT,
    SEND_CHAT_REACT,
    DELETE_CHAT,
    USER_LIST,
    DM, 
    Search_User_When_Create_Room,
    START_PRIVATE_CHAT,
    ACCEPT_PRIVATE_CHAT,
    CREATE_ROOM,
    ENTER_ROOM,
    EXIT_ROOM,
    UPDATE_ROOM_LIST,
    UPDATE_CHAT_LIST,
    CHAT_UPDATED
} packet_type;

typedef enum status_code{
    OK = 200,
    ERROR = 400,
    Session_Not_Found = 401,
    Signup_Already_Exist = 402,
    Login_Failed = 403,

    MYSQL_ERROR = 450,

    INTERNAL_SERVER_ERROR = 500,
}status_code;

typedef struct packet_header_t{
    packet_type type;
    char access_token[64];
    char refresh_token[64];
} packet_header_t;

typedef struct req_packet_t
{
    packet_header_t header;
    char argv[16][64];
} req_packet_t;

typedef struct res_packet_t
{
    packet_header_t header;
    int status;
    int data_len;
    int room_id;
    char msg[128]; // server message
    char id[16];
    char pwd[16];
    char name[16];
    char chat_text[64];
    char data[MAX_PACKET_SIZE]; // 16KB
} res_packet_t;


// static void* dbg_malloc(size_t sz, const char* file, int line) {
//     void* p = malloc(sz);
//     // fprintf(stderr, "[MALLOC] %s:%d → %p (%zu bytes)\n", file, line, p, sz);
//     return p;
// }
// static void dbg_free(void* p, const char* file, int line) {
//     // fprintf(stderr, "[FREE  ] %s:%d → %p\n", file, line, p);
//     free(p);
// }
// #define malloc(sz) dbg_malloc(sz, __FILE__, __LINE__)
// #define free(p)   dbg_free(p, __FILE__, __LINE__)

int is_empty_string(char* str);

char* concat_strings(int count, ...);

char* append_string(char* original, const char* new_str);
#endif