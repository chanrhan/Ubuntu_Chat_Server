#include "gtk_draw.h"
#include "gtk_utils.h"
#include "client_recv_cmd.h"

recv_cmd_t recv_cmd_list[] = {
    {CHAT_UPDATED, recv_cmd_chat_updated}
};

int recv_cmd_list_size = sizeof(recv_cmd_list) / sizeof(recv_cmd_t);

int recv_cmd_chat_updated(res_packet_t* res){
    int room_id = res->room_id;
    printf("CHAT UPDATED : %d, curr:%d\n", room_id, service_page->curr_room_id);

    chat_vo* vo = (chat_vo*)&res->data;

    for(int i=0;i<service_page->room_list_size;++i){
        // printf("saved chats: %s|%s|%d\n", service_page->room_list_data[i].last_chat, service_page->room_list_data[i].last_chat_time, service_page->room_list_data[i].updated_chat_count);
        // printf("___vo chats: %s|%s|%d\n", vo[0].text, vo[0].chat_time, vo[0].non_read_count);
        if(service_page->room_list_data[i].room_id == room_id){
            snprintf(service_page->room_list_data[i].last_chat, 64, "%s", vo[0].text);
            snprintf(service_page->room_list_data[i].last_chat_time, 64, "%s", vo[0].chat_time);
            service_page->room_list_data[i].updated_chat_count = vo[0].non_read_count;
            // service_page->room_list_data[i].member_num = vo[0].reply_chat_id;
            // printf("chat updated(room) : %s|%s|%d\n", service_page->room_list_data[i].last_chat, service_page->room_list_data[i].last_chat_time, service_page->room_list_data[i].updated_chat_count);
            break;
        }
    }
    draw_room_list();

    if(room_id == service_page->curr_room_id){
        // 수신받은 채팅이 현재 입장한 채팅방일 경우
        // 방 목록 + 채팅 목록 모두 업데이트
        reload_chat_list(room_id);
    }
}

int recv_cmd_signup(res_packet_t* res){
    printf("SIGNUP COMPLETE\n");
    gtk_stack_set_visible_child_name(page_stack, "LOGIN");

    return SUCCESS;
}

int recv_cmd_login(res_packet_t* res){
    char* at = res->header.access_token;
    char* id = res->id;
    if(at == NULL || id == NULL || strcmp(id, "") == 0 || strcmp(at, "") == 0){
        printf("Login Success, But Not AccessToken Or Id\n");
        return FAIL;
    }
    
    cli_session.id = strdup(id);
    cli_session.access_token = strdup(at);
    
    if((pthread_create(&sse_thread, NULL, sse_thread_func, NULL)) != 0){
        perror("pthread_create: send");     

        return EXIT_FAILURE;
    }

    gtk_stack_set_visible_child_name(page_stack, "SERVICE");
    reload_service_page();

    return SUCCESS;
}

int recv_cmd_echo(res_packet_t* res_msg){
    printf("Echo: %s\n", res_msg->chat_text);
}

int recv_cmd_chat(res_packet_t* res_msg){
    printf("%s\n", res_msg->chat_text);
}

int recv_cmd_dm(res_packet_t* res_msg){
    printf("[DM] (%s) : %s\n", res_msg->name, res_msg->chat_text);
}

int recv_cmd_user_list(res_packet_t* res_msg){
    
}

int recv_cmd_start_private_chat(res_packet_t* res_msg){
    char* from_client_name = res_msg->name;
    printf("private chat requested by %s (Y / N) : \n", from_client_name);
    

    // int c;
    // while ((c = getchar()) != '\n' && c != EOF);
    return 0;
}

int recv_cmd_room_list(res_packet_t* res_msg){
    
}

int recv_cmd_exit(res_packet_t* res_msg){
    gtk_main_quit();
    exit(EXIT_SUCCESS);
}

void* sse_thread_func(void* arg){
    req_packet_t req = {
        .header.type = SSE_CONNECT
    };
    res_packet_t res2;

    snprintf(req.argv[0], sizeof(req.argv[0]), "%s", cli_session.id);

    printf("[SSE] Try SSE CONNECT : %d\n", sse_fd);

    if(send_and_receive_sse(SSE_CONNECT, &req, &res2) == FAIL){
        perror("send_and_receive");
        return FAIL;
    }
    if(res2.status != OK){
        return FAIL;
    }
    
    printf("[SSE] THREAD START...\n");

    while(1){
        res_packet_t res;
        ssize_t received_len = recv(sse_fd, &res, sizeof(res_packet_t), 0);
    
        if(received_len < 0){
            perror("[SSE] recv");
            break;
        }else if(received_len == 0){
            printf("[SSE] Server closed\n");
            break;
        }else{
            packet_type type = res.header.type;
            int status = res.status;
            printf("[SSE] Receive type : %d\n", type);
            char* msg = res.msg;
            if(status < 400){
                if(strlen(msg) > 0){
                    printf("%s\n", msg);
                }

                for(int i=0;i<recv_cmd_list_size;++i){
                    if(recv_cmd_list[i].type == type && recv_cmd_list[i].cmd_func){
                        recv_cmd_list[i].cmd_func(&res);
                        break;
                    }
                }
            }else{
                if(strlen(msg) > 0){
                    printf("[Error] %s\n", msg);
                }
            }
        }
    }
    printf("SSE THREAD QUIT...\n");
    close(sse_fd);

    pthread_exit(NULL);
}


int send_and_receive(packet_type wait_type, req_packet_t* req, res_packet_t* res){
    if(send(sock_fd, req, sizeof(req_packet_t), 0) < 0){
        perror("send req");
        return FAIL;
    }
    
    ssize_t received_len = recv(sock_fd, res, sizeof(res_packet_t), 0);
    
    if(received_len < 0){
        perror("recv");
        return FAIL;
    }else if(received_len == 0){
        printf("Server closed\n");
        return FAIL;
    }else{
        packet_type type = res->header.type;
        int status = res->status;
        // printf("Receive status : %d\n", status);
        char* msg = res->msg;
        if(status < 400 && type == wait_type){
            if(strlen(msg) > 0){
                printf("%s\n", msg);
            }
            return SUCCESS;
        }else{
            if(strlen(msg) > 0){
                printf("[Error] %s\n", msg);
            }
            return FAIL;
        }
        
    }
}

int send_and_receive_sse(packet_type wait_type, req_packet_t* req, res_packet_t* res){
    if(send(sse_fd, req, sizeof(req_packet_t), 0) < 0){
        perror("send req");
        return FAIL;
    }
    
    ssize_t received_len = recv(sse_fd, res, sizeof(res_packet_t), 0);
    
    if(received_len < 0){
        perror("recv");
        return FAIL;
    }else if(received_len == 0){
        printf("Server closed\n");
        return FAIL;
    }else{
        packet_type type = res->header.type;
        int status = res->status;
        // printf("Receive status : %d\n", status);
        char* msg = res->msg;
        if(status < 400 && type == wait_type){
            if(strlen(msg) > 0){
                printf("%s\n", msg);
            }
            return SUCCESS;
        }else{
            if(strlen(msg) > 0){
                printf("[Error] %s\n", msg);
            }
            return FAIL;
        }
        
    }
}