#include "client.h"

int logined = 0;
int sock_fd;
int sse_fd;
int recv_sock;

client_session cli_session;
pthread_t sse_thread;
pthread_t gtk_thread;

void* gtk_thread_func(void* arg){
    create_gtk_main_window();
    while(1){

    }
    pthread_exit(NULL);
}

int main(int argc, char** argv){
    char buf[BUF_SIZE];

    struct sockaddr_in sin;

    if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);
    sin.sin_addr.s_addr = inet_addr(IP_ADDR);

    if(connect(sock_fd, (struct sockaddr*)&sin, sizeof(sin))){
        perror("connect");
        exit(1);
    }


    struct sockaddr_in sse_sin;

    if((sse_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&sse_sin, 0, sizeof(sse_sin));
    sse_sin.sin_family = AF_INET;
    sse_sin.sin_port = htons(PORT);
    sse_sin.sin_addr.s_addr = inet_addr(IP_ADDR);

    if(connect(sse_fd, (struct sockaddr*)&sse_sin, sizeof(sse_sin))){
        perror("sse connect");
        exit(1);
    }

    gtk_init(&argc, &argv);
    css_init();
    create_gtk_main_window();


    return 0;
}