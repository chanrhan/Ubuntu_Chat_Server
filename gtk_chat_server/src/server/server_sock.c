#include "server_sock.h"
#include "mysql_query.h"

int s_sock = -1;
int cli_sock = -1;
char* buf = NULL;
int client_fds_num = 0;

struct sockaddr_in ser_addr, cli_addr;
socklen_t client_len = sizeof(cli_addr);
pthread_t client_thread = NULL;

struct pollfd client_sock_fds[10];

session_t* session_list = NULL;

pthread_mutex_t mysql_mutex = PTHREAD_MUTEX_INITIALIZER;


int server_socket_init(){
    if((s_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&ser_addr, 0, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(PORT);
    ser_addr.sin_addr.s_addr = inet_addr(IP_ADDR);

    int opt = 1;
    if(setsockopt(s_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsocketopt");
        exit(1);
    }

    if(bind(s_sock, (const struct sockaddr*)&ser_addr, sizeof(ser_addr))){
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if(listen(s_sock, 5)){
        perror("listen");
        exit(1);
    }


    client_sock_fds[0].fd = s_sock;
    client_sock_fds[0].events = POLLIN;
    client_sock_fds[1].fd = STDIN_FILENO;
    client_sock_fds[1].events = POLLIN;
    client_fds_num = 2;
}

char* add_session(int fd, char* id){
    session_t* new_session = malloc(sizeof(session_t));
    if(!new_session){
        printf("[ERROR] new session is NULL\n");
        return NULL;
    }
    new_session->fd = fd;
    new_session->id = strdup(id);
    if(!new_session->id){
        printf("[ERROR] strdup\n");
        return NULL;
    }
    // printf("session id : %s|%s\n", id, new_session->id);

    int expired_time = time(NULL) + SESSION_EXPIRED_TIME;
    // new_session->expired_time = expired_time;

    // create access token
    char* access_token = NULL;
    long exp = 0;
    printf("create access token..\n");
    if((exp = create_jwt(new_session->id, SESSION_EXPIRED_TIME, &access_token)) == FAIL){
        perror("create access token");
        return NULL;
    }
    // new_session->access_token = access_token;

    printf("create refresh token..\n");

    char* refresh_token = NULL;
    if((exp = create_jwt(new_session->id, SESSION_EXPIRED_TIME * 3, &refresh_token)) == FAIL){
        perror("create fresh token"); 
        return NULL;
    }
    if(mysql_refresh_token(new_session->id, refresh_token, exp) == FAIL){
        printf("[ERROR] save refresh token\n");
        return NULL;
    }

    new_session->next = NULL;
    new_session->sse_fd = -1;
    if(session_list == NULL){
        session_list = new_session;
    }else{
        session_t* curr = session_list;

        while(curr->next != NULL){
            curr = curr->next;
        }
        curr->next = new_session;
    }
    // printf("a t : %s\n", access_token);
    // create refresh token

    return access_token;
}

void print_session(){
    printf("---SESSIONS---\n");
    session_t* curr = session_list;

    while(curr != NULL){
        printf("session: %16s\n", curr->id);
        curr = curr->next;
    }
    printf("---------\n");
}

session_t* find_session_by_fd(int fd){
    session_t* curr = session_list;

    while(curr != NULL){
        if(curr->fd == fd){
            break;
        }
        curr = curr->next;
    }
    return curr;
}

session_t* find_session_by_id(char* id){
    session_t* curr = session_list;

    while(curr != NULL){
        if(strcmp(curr->id, id) == 0){
            break;
        }
        curr = curr->next;
    }
    return curr;
}

int find_fd_by_id(char* id){
    session_t* curr = session_list;

    while(curr != NULL){
        if(strcmp(curr->id, id) == 0){
            return curr->fd;
        }
        curr = curr->next;
    }
    return -1;
}

int find_sse_fd_by_id(char* id){
    session_t* curr = session_list;

    while(curr != NULL){
        // printf("find fd : %s\n", curr->id);
        if(strcmp(curr->id, id) == 0){
            return curr->sse_fd;
        }
        curr = curr->next;
    }
    return -1;
}