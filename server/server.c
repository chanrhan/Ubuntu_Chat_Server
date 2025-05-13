/*
    2019136066 박희찬
    실습 : 채팅 서버 (대화방 생성/참여/나가기 기능 구현)
    SERVER

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
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>


#define PORT 9000
#define BUF_SIZE 1024
#define IP_ADDR "127.0.0.1"

#define MAX_CLIENT 10
#define FAIL 1
#define SUCCESS 0

#define DEFAULT_CHATROOM_NUM 0

#define MSG_SIZE 100
#define DATA_SIZE 2048
#define ERR_STATUS 400


int client_fds_num = 1;

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

typedef struct req_msg_t{
    cmd_type cmd;
    int argc;
    char argv[4][8];
} req_msg_t;

typedef int (*cmd_func_t)(int fd, req_msg_t req_msg);

#define DECLARE_CMDFUNC(str) int cmd_##str(int fd, req_msg_t req_msg)

DECLARE_CMDFUNC(signup);
DECLARE_CMDFUNC(login);
DECLARE_CMDFUNC(echo);
DECLARE_CMDFUNC(chat);
DECLARE_CMDFUNC(dm);
DECLARE_CMDFUNC(user_list);
DECLARE_CMDFUNC(start_private_chat);
DECLARE_CMDFUNC(accept_private_chat);
DECLARE_CMDFUNC(create_room);
DECLARE_CMDFUNC(enter_room);
DECLARE_CMDFUNC(exit_room);
DECLARE_CMDFUNC(room_list);

int receive_cmd(int fd, req_msg_t* req_msg);

typedef struct cmd_func_pair_t{
    cmd_type cmd;
    cmd_func_t cmd_func;
} cmd_func_pair_t;

typedef struct member_t{
    int chatroom_id;
    int client_fd;
}member_t;

chatroom_t room_list[32] = {
    {0, "DEFAULT", 1}
};
int room_list_size = 1;

member_t member_list[128];
int member_list_size = 0;

cmd_func_pair_t cmd_func_list[] = {
    {SIGNUP, cmd_signup},
    {LOGIN, cmd_login},
    {ECHO, cmd_echo},
    {CHAT, cmd_chat},
    {DM, cmd_dm},
    {USER_LIST, cmd_user_list},
    {START_PRIVATE_CHAT, cmd_start_private_chat},
    {CREATE_ROOM, cmd_create_room},
    {ENTER_ROOM, cmd_enter_room},
    {EXIT_ROOM, cmd_exit_room},
    {ROOM_LIST, cmd_room_list}
};

int cmd_list_size = sizeof(cmd_func_list) / sizeof(cmd_func_pair_t);

client_info_t client_db[64] = {
    {0,0,0, "a", "0000", "AAA"},
    {0,0,0, "b", "0000", "BBB"}
};
int client_db_size = 2;

client_info_t* curr_client_list[MAX_CLIENT];
int curr_client_list_size = 0;

char private_chat_wait_req_list[32][32];
int private_chat_wait_req_list_size = 0;

// 인자로 넘겨준 변수는 C에서 call by value이므로, 함수 내에서 포인터 변수에 값을 할당해도 외부에 적용되진 않는다!
// 따라서 이중 포인터로 선언하여 값이 반영될 수 있도록 한다 
int get_client_info(int fd, client_info_t** out_client){
    for(int i=0;i<curr_client_list_size;++i){
        client_info_t* cli = curr_client_list[i];
        if(cli->fd == fd){
            *out_client = cli;
            return SUCCESS;
        }
    }
    return FAIL;
}

int find_fd_by_client_id(char* id){
    for(int i=0;i<curr_client_list_size;++i){
        // printf("curr id : %s, id : %s\n", curr_client_list[i]->id, id);
        if(strcmp(curr_client_list[i]->id, id) == 0){
            return curr_client_list[i]->fd;
        }
    }
    return -1;
}

int check_login_duplicated(char* id){
    for(int i=0;i<curr_client_list_size;++i){
        if(strcmp(curr_client_list[i]->id,id) == 0){
            return i;
        }
    }
    return -1;
}

int check_id_duplicated(char* id){
    for(int i=0;i<client_db_size;++i){
        if(strcmp(client_db[i].id, id) == 0){
            return i;
        }
    }
    return -1;
}

// chatroom
#pragma region chatroom
int get_chatroom_info(int chatroom_id, chatroom_t** chatroom){
    for(int i=0;i<room_list;++i){
        chatroom_t* c = &room_list[i];
        if(c->id == chatroom_id){
            *chatroom = c;
            return SUCCESS;
        }
    }
    return FAIL;
}

int find_client_in_chatroom(int chatroom_id, int client_fd, client_info_t** out_cli){
    for(int i=0;i<member_list_size;++i){
        member_t* c = &member_list[i];
        if(c->chatroom_id == chatroom_id && c->client_fd == client_fd){
            *out_cli = c;
            return SUCCESS;
        }
    }
    return FAIL;
}

// 나중에 fd -> id로 수정할것 
// + chatroom_list 는 chatroom_t* 로 관리하도록 수정할것 (chatroom_t 내에 client[] 정보 넣을 것인지는 아직 미정)
int create_chatroom(char* room_name, int is_private, int fds_size, int init_fds[]){
    int room_id = room_list_size + (is_private == 0 ? 100 : 0);
    chatroom_t* room = &room_list[room_list_size];
    room->id = room_id;
    snprintf(room->name, 8, "%s", room_name);
    room->is_private = is_private;

    if(fds_size > 0 && init_fds != NULL){
        client_info_t* cli;
        for(int i=0;i<fds_size;++i){
            if(get_client_info(init_fds[i], &cli) < 0){
                continue;
            }
            member_list[member_list_size].chatroom_id = room_id;
            member_list[member_list_size].client_fd = init_fds[i];
            cli->room_id = room_id;
            
            ++member_list_size;
        }
    }
    ++room_list_size;
    return SUCCESS;
}

int enter_chatroom(int chatroom_id, int client_fd){
    client_info_t* cli = NULL;
    if(find_client_in_chatroom(chatroom_id, client_fd, &cli) == FAIL){
        if(get_client_info(client_fd, &cli) == SUCCESS){
            cli->room_id = chatroom_id;
            member_list[member_list_size].chatroom_id = chatroom_id;
            member_list[member_list_size].client_fd = client_fd;
            member_list_size++;
            return SUCCESS;
        }
        
    }
    return FAIL;
}

int exit_chatroom(int client_fd){
    client_info_t* cli = NULL;
    if(get_client_info(client_fd, &cli) == SUCCESS){
        int exit_room_id = cli->room_id;
        cli->room_id = -1;
        for(int i=0;i<member_list_size;++i){
            member_t* member = &member_list[i];
            if(member->chatroom_id == exit_room_id && member->client_fd == client_fd){
                member->chatroom_id = -1;
                member->client_fd = -1;
                return SUCCESS;
            }
        }
        return FAIL;
    }
    return FAIL;
}
#pragma endregion

int print_client_info(client_info_t cli){
    printf("(client)[%c] nickname: %s, id: %s, pwd: %s\n",(cli.authorized ? 'O':'X') , cli.name, cli.id, cli.pwd);
}

int send_to_client(int fd, res_msg_t* res_msg){
    if(send(fd, res_msg, sizeof(res_msg_t), 0) < 0){
        perror("send");
        return FAIL;
    }
    return SUCCESS;
}

int send_text(int fd, char* text){
    res_msg_t res_msg = {
        .status = OK
    };
    strncpy(res_msg.msg, text, strlen(text));
    // printf("send text to [%d] : %s\n", fd, (char*)&res_msg.buf);
    return send_to_client(fd, &res_msg);
}

int send_error(int fd, char* text){
    res_msg_t res_msg = {
        .status = ERROR
    };
    strncpy(&res_msg.msg, text, strlen(text));
    return send_to_client(fd, &res_msg);
}

// 회원가입 
int cmd_signup(int fd, req_msg_t req_msg){
    char* id = req_msg.argv[0];
    char* pwd = req_msg.argv[1];
    char* name = req_msg.argv[2];
    printf("[%d] (signup) id: %s, pwd: %s, name: %s\n", fd, id, pwd, name);

    if(check_id_duplicated(id) >= 0){
        send_error(fd, "Duplicated id");
        return FAIL;
    }
    
    client_info_t new_client;
    
    // new_client.fd = fd;
    snprintf(new_client.name, sizeof(new_client.name), "%s", name);
    snprintf(new_client.id, sizeof(new_client.id), "%s", id);
    snprintf(new_client.pwd, sizeof(new_client.pwd), "%s", pwd);

    memcpy(&client_db[client_db_size], &new_client, sizeof(client_info_t));
    // printf("(signup) username: %s, pwd: %s, nickname: %s\n", username, password, nickname);
    send_text(fd, "signup completed!");
    client_db_size++;
    return SUCCESS;
}

// 클라이언트 추가 (접속 시)
int add_client(int fd){
    client_info_t cli;
    cli.fd = fd;
    cli.authorized = 0;
    curr_client_list[curr_client_list_size] = (client_info_t*)malloc(sizeof(client_info_t));

    memcpy(curr_client_list[curr_client_list_size], &cli, sizeof(client_info_t));
   
    curr_client_list_size++;

    return SUCCESS;
}

// 로그인 
int cmd_login(int fd, req_msg_t req_msg){
    char* id = req_msg.argv[0];
    char* pwd = req_msg.argv[1];
    printf("[%d] (login) id: %s, pwd: %s\n", fd, id, pwd);

    client_info_t* cli;
    if(get_client_info(fd, &cli) < 0){
        send_error(fd, "Login Failed");
        return FAIL;
    }

    if(cli->authorized == 1){
        send_error(fd, "Already logined");
        return FAIL;
    }
    
    for(int i=0;i<client_db_size;++i){
        // check id & password 
        if(strcmp(client_db[i].id, id) == 0 && strcmp(client_db[i].pwd, pwd) == 0){
            if(check_login_duplicated(id) >= 0){
                send_error(fd, "Duplicated Login Access!");
                return FAIL;
            }
            snprintf(cli->id, 8, "%s",  id);
            snprintf(cli->name, 8, "%s", client_db[i].name);
            snprintf(cli->pwd, 8, "****");
            cli->authorized = 1; // 인증 완료 

            enter_chatroom(DEFAULT_CHATROOM_NUM, fd);
            res_msg_t res_msg = {
                .status = LOGIN
            };
            char* text = "Logined!";
            snprintf(res_msg.msg, strlen(text), "%s", text);
            send_to_client(fd, &res_msg);

            return SUCCESS;
        }
    }
    send_text(fd, "Login failed!");
    return FAIL;
}

// 에코 
int cmd_echo(int fd, req_msg_t req_msg){
    printf("[%d] (echo) text : %s\n", fd, req_msg.argv[0]);
    res_msg_t res_msg = {
        .status = ECHO
    };
    snprintf(res_msg.chat_text, sizeof(res_msg.chat_text), "%s", req_msg.argv[0]);
    return send_to_client(fd, &res_msg);
}

// 채팅 전송 
int cmd_chat(int fd, req_msg_t req_msg){
    char* text = req_msg.argv[0];
    printf("[%d] (chat) text : %s\n", fd, text);

    client_info_t* sender_cli = NULL;
    if(get_client_info(fd, &sender_cli) < 0){
        return send_error(fd, "chat failed");
    }
    int curr_room_id = sender_cli->room_id;
    if(curr_room_id < 0){
        return send_error(fd, "please enter chatroom first");
    }
    chatroom_t* chatroom_info;
    if(get_chatroom_info(curr_room_id, &chatroom_info) == FAIL){
        send_error(fd, "chatroom not found");
        return FAIL;
    }
    
    for(int i=0;i<member_list_size;++i){
        member_t* c = &member_list[i];
        if(c->chatroom_id == curr_room_id && c->client_fd != fd){
            client_info_t* recv_cli;
            if(get_client_info(c->client_fd, &recv_cli) == FAIL){
                continue;
            }
            if(recv_cli->room_id != curr_room_id){
                continue;
            }
            res_msg_t res_msg = {
                .status = CHAT,
                .msg = ""
            };
            snprintf(&res_msg.chat_text, sizeof(res_msg.chat_text), "[%s] (%s) : %s", chatroom_info->name, recv_cli->name, text);
            send_to_client(c->client_fd, &res_msg);
        }
    }
    return SUCCESS;
}

// 귓속말 
int cmd_dm(int fd, req_msg_t req_msg){
    char* target_id = req_msg.argv[0];
    char* text = req_msg.argv[1];
    printf("[%d](dm) to : %s, text : %s\n", target_id, text);

    client_info_t* cli;
    if(get_client_info(fd, &cli) < 0){
        return FAIL;
    }
    printf("cli : %s\n", cli->id);

    int target_fd=0;
    if((target_fd = find_fd_by_client_id(target_id)) < 0){
        return FAIL;
    }
    printf("target fd : %d\n", target_fd);

    res_msg_t res_msg = {
        .status = DM
    };

    snprintf(res_msg.id, sizeof(res_msg.id), "%s", cli->id);
    snprintf(res_msg.name, sizeof(res_msg.name), "%s", cli->name);
    snprintf(res_msg.chat_text, sizeof(res_msg.chat_text), "%s", text);

    return send_to_client(target_fd, &res_msg);
}

// 멤버 리스트 
int cmd_user_list(int fd, req_msg_t req_msg){
    printf("member_list\n");

    res_msg_t res_msg ={
        .status = USER_LIST,
        .msg = ""
    };

    int count = 0;
    
    for(int i=0;i<curr_client_list_size;++i){
        if(curr_client_list[i]->authorized == 1){
            memcpy(&res_msg.clients[count], curr_client_list[i], sizeof(client_info_t));
            ++count;
        }
    }
    res_msg.clients_len = count;

    return send_to_client(fd, &res_msg);
}

int wait_private_chat_req(char* id){
    snprintf(private_chat_wait_req_list[private_chat_wait_req_list_size], 32, "%s", id);
    ++private_chat_wait_req_list_size;
    return SUCCESS;
}

int find_private_chat_wait_request(char* id){
    for(int i=0;i<private_chat_wait_req_list_size;++i){
        if(strcmp(private_chat_wait_req_list[i], id) == 0){
            return SUCCESS;
        }
    }
    return FAIL;
}

// 1:1 대화 요청 
int cmd_start_private_chat(int fd, req_msg_t req_msg){
    char* to_client_id = req_msg.argv[0];
    printf("[%d] (start_private_chat) to : %s\n", fd, to_client_id);
    client_info_t* cli;
    if(get_client_info(fd, &cli) < 0){
        return FAIL;
    }

    int target_fd;
    if((target_fd = find_fd_by_client_id(to_client_id)) < 0){
        return FAIL;
    }
    res_msg_t res_msg = {
        .status = START_PRIVATE_CHAT
    };
    // snprintf(res_msg.msg, sizeof(res_msg.msg), "request 1:1 chat by %s : (Y/N)", cli->name);
    snprintf(res_msg.id, sizeof(res_msg.id), "%s", cli->id);
    snprintf(res_msg.name, sizeof(res_msg.name), "%s", cli->name);

    wait_private_chat_req(cli->id);

    send_to_client(target_fd, &res_msg);
    
    return SUCCESS;
}

// 1:1 대화 요청 수락 
int cmd_accept_private_chat(int fd, req_msg_t req_msg){
    char accept = req_msg.argv[0][0];
    char* owner_client_id = req_msg.argv[1];
    printf("[%d] (accept_private_chat) owner_id : %s, accept: %c\n",fd, owner_client_id, accept);
    if(find_private_chat_wait_request(owner_client_id) == FAIL){
        perror("find_private_chat_wait_request");
        return FAIL;
    }

    client_info_t* from_client, *to_client;
    int from_client_fd;
    if((from_client_fd = find_fd_by_client_id(owner_client_id)) < 0){
        perror("find_fd_by_client_id");
        return FAIL;
    }
    if(get_client_info(from_client_fd, &from_client) < 0){
        perror("get_client_info : from");
        return FAIL;
    }

    if(get_client_info(fd, &to_client) < 0){
        perror("get_client_info : to");
        return FAIL;
    }

    if(accept == '0'){
        printf("[%d] (accept_private_chat) accepted\n", fd);

        char* room_name = (char*)malloc(8);
        snprintf(room_name, 8, "P_%s_%s", from_client->name, to_client->name);
        int init_fds[2];
        init_fds[0] = from_client_fd;
        init_fds[1] = fd;
        create_chatroom(room_name, 0, 2, init_fds);

        send_text(from_client_fd, "private chat opened!");
        send_text(fd, "private chat opened!");
    }else{
        printf("[%d] (start_private_chat) denied\n", fd);
        send_text(from_client_fd, "private chat denied!");
    }
}

int cmd_create_room(int fd, req_msg_t req_msg){
    char* room_name = &req_msg.argv[0];
    printf("[%d] (create_room) room_name : %s\n",fd, room_name);

    if(create_chatroom(room_name, 1, 0, NULL) == FAIL){
        send_error(fd, "fail to create room");
        return FAIL;
    }

    send_text(fd, "room created!");
    return SUCCESS;
}

int cmd_enter_room(int fd, req_msg_t req_msg){
    int room_num = atoi(req_msg.argv[0]);
    printf("[%d] (enter_room) room_num : %d\n",fd, room_num);

    if(enter_chatroom(room_num, fd) == FAIL){
        send_error(fd, "fail to enter room");
        return FAIL;
    }
    send_text(fd, "entered room");
    return SUCCESS;
}

int cmd_exit_room(int fd, req_msg_t req_msg){
    printf("[%d] (exit_room) \n",fd);

    if(exit_chatroom(fd) == FAIL){
        send_error(fd, "fail to exit room");
        return FAIL;
    }

    send_text(fd, "exit room");
    return SUCCESS;
}

int cmd_room_list(int fd, req_msg_t req_msg){
    printf("[%d] (room_list) \n",fd);
    
    res_msg_t res_msg = {
        .status = ROOM_LIST
    };
    int count = 0;
    for(int i=0;i<room_list_size;++i){
        chatroom_t* room = &room_list[i];
        if(room->is_private == 1){
            memcpy(&res_msg.rooms[count], room, sizeof(chatroom_t));
            ++count;
        }
    }
    res_msg.room_len = count;
    send_to_client(fd, &res_msg);
    return SUCCESS;
}



int receive_cmd(int fd, req_msg_t* req_msg){
    ssize_t received_len = recv(fd, req_msg, sizeof(req_msg_t), 0);

    if(received_len < 0){
        perror("recv");
        return FAIL;
    }else if(received_len == 0){
        printf("Server closed\n");
        return FAIL;
    }

    return SUCCESS;
}

int execute_cmd(int fd, req_msg_t req_msg){
    for(int i=0;i<cmd_list_size;++i){
        if(cmd_func_list[i].cmd == req_msg.cmd){
            cmd_func_list[i].cmd_func(fd, req_msg);
            return SUCCESS;
        }
    }
    return FAIL;
}

void* client_thread_func(void* arg){
    int fd = *((int*)arg);
    printf("Client Joined: %d\n", fd);
    
    req_msg_t req_msg;

    while(1){
        if(receive_cmd(fd, &req_msg) < 0){
            perror("receive_cmd");
            break;
        }
        if(execute_cmd(fd, req_msg) < 0){
            perror("execute_cmd");
            break;
        }
    }

    pthread_exit(NULL);
}


int main(){
    int s_sock, cli_sock;
    char* buf = (char*)malloc(BUF_SIZE);

    struct sockaddr_in ser_addr, cli_addr;
    socklen_t client_len = sizeof(cli_addr);

    pthread_t client_thread;

    // poll
    struct pollfd client_sock_fds[10];
    

    #pragma region 'server initialize'
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
    #pragma endregion


    client_sock_fds[0].fd = s_sock;
    client_sock_fds[0].events = POLLIN;
    client_sock_fds[1].fd = STDIN_FILENO;
    client_sock_fds[1].events = POLLIN;
    client_fds_num = 2;

    while (1)
    {
        // usleep(10000);
        // printf("----read wait----\n");

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

                    add_client(cli_sock);
                    
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
                for(int c=0;c<curr_client_list_size;++c){
                    client_info_t* cli = curr_client_list[c];
                    print_client_info(*cli);
                }
            }
            fflush(stdout);
        }

        
    }

    close(cli_sock);
    close(s_sock);
    

    return 0;
}