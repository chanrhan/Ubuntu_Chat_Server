#include "server.h"

void* client_thread_func(void* arg){
    int fd = *((int*)arg);
    printf("Client Joined: %d\n", fd);
    mysql_thread_init();

    while(1){
        if(receive_cmd(fd) < 0){
            perror("receive_cmd");
            break;
        }
        // if(execute_cmd(fd, req_msg) < 0){
        //     perror("execute_cmd");
        //     break;
        // }
    }

    close(fd);
    mysql_thread_end();

    pthread_exit(NULL);
}
 
int main(){
    buf = (char*)malloc(BUF_SIZE);

    server_socket_init();
    if (mysql_connect() == FAIL) {
        fprintf(stderr, "MySQL connection failed.\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        if((poll(client_sock_fds, client_fds_num, -1)) < 0){
            perror("poll");
            break;
        }

        // server fd 검사 = 클라이언트 접속 관리 
        if(client_sock_fds[0].revents & POLLIN){
            struct sockaddr_in cli_addr;
            socklen_t client_len = sizeof(cli_addr);

            cli_sock = accept(s_sock, (struct sockaddr*)&cli_addr, &client_len);
            if(cli_sock < 0){
                perror("accept");
            }else{
                if(client_fds_num < MAX_CLIENT + 1){
                    
                    client_sock_fds[client_fds_num].fd = cli_sock;
                    client_sock_fds[client_fds_num].events = POLLIN;
                    client_sock_fds[client_fds_num].revents = 0;

                    // add_client(cli_sock);
                    
                    client_fds_num++;

                    int* p = malloc(sizeof(int));
                    *p = cli_sock;

                    if((pthread_create(&client_thread, NULL, client_thread_func, (void*)p)) != 0){
                        perror("pthread_create: send");     

                        return EXIT_FAILURE;
                    }
                }
            }
        }else{
            // 서버 명령어 입력 단 
            
            fgets(buf, BUF_SIZE, stdin);
            strtok(buf, " \n");

            if(strcmp(buf, "/list") == 0){
                // printf("cli size: %d\n", curr_client_list_size);
                // for(int c=0;c<curr_client_list_size;++c){
                //     client_t* cli = connected_clients[c];
                //     print_client_info(*cli);
                // }
            }
            fflush(stdout);
        }

        
    }

    // close(cli_sock);
    close(s_sock);


    return 0;
}