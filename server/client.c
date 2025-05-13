/*
    2019136066 박희찬
    실습 : 채팅 서버 (대화방 생성/참여/나가기 기능 구현)
    CLIENT

    --명령어 목록--
    /signup [id] [pwd] [name] : 회원가입 
    /login [id] [pwd] : 로그인
    /echo [text] : 에코
    /dm [target_id] [text] : [target_id]에 해당하는 유저에게 메세지 전송
    /chat [text] : 현재 채팅방에 메세지 전송
    /members : 현재 접속중인 유저 목록 출력 
    /start [target_id] : [target_id]에 해당하는 유저와 1:1 채팅방 생성 
    (1:1 대화 수락은 client에서 (y/n) 을 누르면 된다)

    /new [room_name] : 새로운 채팅방 생성
    /enter [room_id] : [room_id]에 해당하는 채팅방 들어가기
    /exit : 현재 채팅방에서 나가기 
    /chats : 채팅방 목록 출력 

    --도움말--
    테스트 중 로그인 시에는 사전 정의된 id/pwd 를 사용하기 바람
    (1) id : a / pwd : 0000 / name : AAA
    (2) id : b / pwd : 0000 / name : BBB
*/

#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MSG_SIZE 100
#define DATA_SIZE 2048
#define ERR_STATUS 400

typedef enum cmd_type{
    SIGNUP = 10,
    LOGIN,
    ECHO,
    CHAT,
    USER_LIST,
    DM,
    START_PRIVATE_CHAT,
    ACCEPT_PRIVATE_CHAT,
    CREATE_ROOM,
    ENTER_ROOM,
    EXIT_ROOM,
    ROOM_LIST,
    OK = 200,
    ERROR = 400
} cmd_type;

typedef struct client_info_t
{
    int fd;
    int authorized;
    int room_id;
    char id[8];
    char pwd[8];
    char name[8];
} client_info_t;

typedef struct chatroom_t{
    int id;
    char name[8];
    int is_private; // 0 : private , 1 : public
} chatroom_t;

typedef struct res_msg_t
{
    cmd_type status;
    int clients_len;
    int room_len;
    char msg[MSG_SIZE]; // server message
    char id[8];
    char password[8];
    char name[8];
    char chat_text[64];
    client_info_t clients[32];
    chatroom_t rooms[32];
} res_msg_t;

typedef int (*req_cmd_func_t)(res_msg_t* res_msg);

typedef struct req_msg_t{
    cmd_type cmd;
    int argc;
    char argv[4][8];
} req_msg_t;

typedef struct req_cmd_t{
    char cmd_str[16];
    cmd_type cmd;
    int required_argc;
    char err_msg[32];
    req_cmd_func_t cmd_func;
}req_cmd_t;

#define DECLARE_CMDFUNC(str) int cmd_##str(res_msg_t* res_msg)

DECLARE_CMDFUNC(signup);
DECLARE_CMDFUNC(login);
DECLARE_CMDFUNC(echo);
DECLARE_CMDFUNC(chat);
DECLARE_CMDFUNC(dm);
DECLARE_CMDFUNC(user_list);
DECLARE_CMDFUNC(start_private_chat);
DECLARE_CMDFUNC(room_list);

req_cmd_t req_cmd_lsit[] = {
    {"signup", SIGNUP, 3, "", cmd_signup},
    {"login", LOGIN, 2, "", cmd_login},
    {"echo", ECHO, 1, "" , cmd_echo},
    {"chat", CHAT, 1, "", cmd_chat},
    {"dm", DM, 2, "", cmd_dm},
    {"users", USER_LIST, 0, "", cmd_user_list},
    {"start", START_PRIVATE_CHAT, 1, "", cmd_start_private_chat},
    {"new", CREATE_ROOM, 1, "", NULL},
    {"enter", ENTER_ROOM, 1, "", NULL},
    {"exit", EXIT_ROOM, 0, "", NULL},
    {"chats", ROOM_LIST, 0, "", cmd_room_list},
};

int req_cmd_lsit_size = sizeof(req_cmd_lsit) / sizeof(req_cmd_t);


#define PORT 9000
#define BUF_SIZE 1024

#define IP_ADDR "127.0.0.1"

int logined = 0;
int sd;
char buf[BUF_SIZE];

int select_cmd(char* cmd_str, int argc){
    for(int i=0;i<req_cmd_lsit_size;++i){
        req_cmd_t* rc = &req_cmd_lsit[i];
        if(strcmp(rc->cmd_str, cmd_str) == 0){
            if(rc->required_argc != argc){
                printf("%s\n", rc->err_msg);
                return -1;
            }
            return req_cmd_lsit[i].cmd;
        }
    }
    return -1;
}


int cmd_signup(res_msg_t* res_msg){

}

int cmd_login(res_msg_t* res_msg){
    logined = 1;
}

int cmd_echo(res_msg_t* res_msg){
    printf("Echo: %s\n", res_msg->chat_text);
}

int cmd_chat(res_msg_t* res_msg){
    printf("%s\n", res_msg->chat_text);
}

int cmd_dm(res_msg_t* res_msg){
    printf("[DM] (%s) : %s\n", res_msg->name, res_msg->chat_text);
}

int cmd_user_list(res_msg_t* res_msg){
    printf("|%4s |%4s |%10s |\n", "NO", "ID", "NAME");
    for(int i=0;i<res_msg->clients_len;++i){
        printf("|%4d |%4s |%10s |\n", i, res_msg->clients[i].id, res_msg->clients[i].name);
    }
}

int cmd_start_private_chat(res_msg_t* res_msg){
    char* from_client_name = res_msg->name;
    printf("private chat requested by %s (Y / N) : \n", from_client_name);
    
    if(fgets(buf, 2, stdin) == NULL){
        puts("fgets error");
        return -1;
    }
    buf[strcspn(buf, "\n")] = '\0';


    printf("y/n : %s\n", buf);

    int accept = 1;
    if(buf[0] == 'Y' || buf[0] == 'y'){
        accept = 0;
    }else if(buf[0] == 'N' || buf[0] == 'n'){
        accept = 1;
    }else{
        accept = -1;
    }

    req_msg_t req_msg = {
        .cmd = ACCEPT_PRIVATE_CHAT
    };
    req_msg.argc = 1;
    snprintf(req_msg.argv[0], sizeof(req_msg.argv[0]), "%d", accept);
    snprintf(req_msg.argv[1], sizeof(req_msg.argv[1]), "%s", res_msg->id);

    printf("send\n");

    if(send(sd, (void*)&req_msg, sizeof(req_msg_t), 0) < 0){
        perror("send");
        return -1;
    }

    // int c;
    // while ((c = getchar()) != '\n' && c != EOF);
    return 0;
}

int cmd_room_list(res_msg_t* res_msg){
    printf("|%8s |%8s |\n", "ROOM_ID", "NAME");
    for(int i=0;i<res_msg->room_len;++i){
        printf("|%8d |%8s |\n", res_msg->rooms[i].id, res_msg->rooms[i].name);
    }
}


int main(){
    client_info_t my_client;

    struct sockaddr_in sin;

    if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // struct timeval optval = {3, 500000};
    // int retval = setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &optval, sizeof(optval));

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);
    sin.sin_addr.s_addr = inet_addr(IP_ADDR);

    if(connect(sd, (struct sockaddr*)&sin, sizeof(sin))){
        perror("connect");
        exit(1);
    }

    pid_t pid = fork();

    char* cmd_str = (char*)malloc(10);

    if(pid < 0){
        perror("fork");
    }else if(pid == 0){
        // Children
        while(1){
            // printf("Send: ");
            
            
            req_msg_t req_msg = {
                .cmd = 0,
                .argc = 0
            };
            if(fgets(buf, BUF_SIZE, stdin) == NULL){
                puts("fgets error");
                break;
            }
            buf[strcspn(buf, "\n")] = '\0';

            char* tok = strtok(buf, " ");
            snprintf(cmd_str, 8, "%s", tok);

            int argc = 0;
            for (;(tok = strtok(NULL, " ")) != NULL;++argc)
            {
                // req_msg.argv[argc] = (char*)malloc(20);
                snprintf(req_msg.argv[argc], 8, "%s", tok);
            }
            // cmd
            if(cmd_str[0] != '/'){
                continue;
            }

            cmd_str = cmd_str + 1;
            int cmd;

            if((cmd = select_cmd(cmd_str, argc)) < 0){
                printf("Invalid command : %s\n", cmd_str);
                continue;
            }

            req_msg.cmd = cmd;
            req_msg.argc = argc;

            if(send(sd, (void*)&req_msg, sizeof(req_msg_t), 0) < 0){
                perror("send");
                break;
            }

            // int c;
            // while ((c = getchar()) != '\n' && c != EOF);
        }
    }else{
        // Parent
        while(1){
            res_msg_t res_msg;
            ssize_t received_len = recv(sd, &res_msg, sizeof(res_msg_t), 0);
            
            if(received_len < 0){
                perror("recv");
                break;
            }else if(received_len == 0){
                printf("Server closed\n");
                break;
            }else{
                cmd_type status = res_msg.status;
                // printf("Receive status : %d\n", status);
                char* msg = res_msg.msg;
                if(status < ERR_STATUS){
                    if(strlen(msg) > 0){
                        printf("%s\n", msg);
                    }

                    for(int i=0;i<req_cmd_lsit_size;++i){
                        if(req_cmd_lsit[i].cmd == status && req_cmd_lsit[i].cmd_func){
                            req_cmd_lsit[i].cmd_func(&res_msg);
                        }
                    }
                }else{
                    if(strlen(msg) > 0){
                        printf("[Error] %s\n", msg);
                    }
                }
                
            }
        }
    }

    close(sd);

    return 0;
}