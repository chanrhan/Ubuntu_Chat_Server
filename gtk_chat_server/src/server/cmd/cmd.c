#include "server_cmd.h"
#include "mysql_query.h"

cmd_func_pair_t cmd_func_list[] = {
    {SIGNUP, cmd_signup},
    {LOGIN, cmd_login},
    {SSE_CONNECT, cmd_sse_connect},
    {ECHO, cmd_echo},
    {SEND_CHAT, cmd_chat},
    {SEND_CHAT_REACT, cmd_chat_react},
    {DELETE_CHAT, cmd_delete_chat},
    {USER_LIST, cmd_user_list},
    {CREATE_ROOM, cmd_create_room},
    {ENTER_ROOM, cmd_enter_room},
    {EXIT_ROOM, cmd_exit_room},
    {UPDATE_ROOM_LIST, cmd_room_list},
    {Search_User_When_Create_Room, cmd_search_user_when_create_room},
    {UPDATE_CHAT_LIST, cmd_update_chat_list}
};

int cmd_list_size = sizeof(cmd_func_list) / sizeof(cmd_func_pair_t);

int receive_cmd(int fd){
    req_packet_t req;
    ssize_t received_len = recv(fd, &req, sizeof(req_packet_t), 0);

    if(received_len < 0){
        perror("recv");
        return FAIL;
    }else if(received_len == 0){
        printf("Server closed\n");
        return FAIL;
    }else{
        execute_cmd(fd, req);
    }

    return SUCCESS;
}

int execute_cmd(int fd, req_packet_t req){
    // printf("recv type : %d\n", req.header.type);
    for(int i=0;i<cmd_list_size;++i){
        if(cmd_func_list[i].cmd == req.header.type){
            cmd_func_list[i].cmd_func(fd, req);
            return SUCCESS;
        }
    }
    return FAIL;
}


int send_to_client(int fd, res_packet_t* res_msg){
    if(send(fd, res_msg, sizeof(res_packet_t), 0) < 0){
        perror("send");
        return FAIL;
    }
    return SUCCESS;
}

// int send_text(int fd, char* text){
//     res_packet_t res_msg = {
//         .status = OK
//     };
//     strncpy(res_msg.msg, text, strlen(text));
//     // printf("send text to [%d] : %s\n", fd, (char*)&res_msg.buf);
//     return send_to_client(fd, &res_msg);
// }

int send_packet(int fd, packet_type type, status_code status, char* text){
    res_packet_t res_msg = {
        .header = {
            .type = type
        },
        .status = status
    };
    strncpy(&res_msg.msg, text, strlen(text));
    return send_to_client(fd, &res_msg);
}

int send_chat_in_room(int room_id){
    chat_vo* user_list = NULL;
    int len = mysql_get_inner_room_members(room_id, &user_list);

    for(int i=0;i<len;++i){
        // printf("[%d] %s|%s|%d\n", room_id, user_list[i].owner_id, user_list[i].chat_time, user_list[i].non_read_count);
        // print_session();
        int target_fd = find_sse_fd_by_id(user_list[i].owner_id);
        if(target_fd <= 0){
            continue;
        }
        
        res_packet_t res = {
            .header.type = CHAT_UPDATED,
            .status = OK
        };

        res.room_id = room_id;
        memcpy(res.data, &user_list[i], sizeof(chat_vo));
        res.data_len = 1;
        // printf("target fd: %d\n", target_fd);
        if(send_to_client(target_fd, &res) == FAIL){
            user_list = NULL;
            return FAIL;
        }
    }

    user_list = NULL;
    return SUCCESS;
}

// 회원가입 
int cmd_signup(int fd, req_packet_t req){
    user_vo vo;
    snprintf(vo.id, sizeof(vo.id), "%s", req.argv[0]);
    snprintf(vo.pwd, sizeof(vo.pwd), "%s", req.argv[1]);
    snprintf(vo.name, sizeof(vo.name), "%s", req.argv[2]);

    printf("[%d] (signup) id: %s, pwd: %s, name: %s\n", fd, vo.id, vo.pwd, vo.name);

    if(mysql_exist_id(vo.id) < 0){
        send_packet(fd, SIGNUP, Signup_Already_Exist, "이미 존재하는 아이디입니다.");
        return FAIL;
    }

    if(mysql_insert_user(&vo) < 0){
        send_packet(fd, SIGNUP, INTERNAL_SERVER_ERROR, "[MYSQL] INSERT ERROR");
        return FAIL;
    }


    send_packet(fd, SIGNUP, OK, "Singup Success");


    return SUCCESS;
}


// 로그인 
int cmd_login(int fd, req_packet_t req){
    user_vo vo;
    snprintf(vo.id, sizeof(vo.id), "%s", req.argv[0]);
    snprintf(vo.pwd, sizeof(vo.pwd), "%s", req.argv[1]);
    
    printf("[%d] (login) id: %s, pwd: %s\n", fd, vo.id, vo.pwd);

    if(mysql_match_user(&vo) < 0){
        send_packet(fd, LOGIN, Login_Failed, "Login Failed");
        return FAIL;
    }
    char* access_token = add_session(fd, vo.id);
    print_session();

    res_packet_t res = {
        .header = {
            .type = LOGIN,
            .access_token = access_token
        },
        .status = OK,
    };
    snprintf(res.id, sizeof(res.id), vo.id);
    snprintf(res.chat_text, sizeof(res.chat_text), "Login Success");
    send_to_client(fd, &res);

    return SUCCESS;
}

// SSE 연결 
int cmd_sse_connect(int fd, req_packet_t req){
    printf("[%d] SSE CONNECT : (%s)\n", fd, req.argv[0]);
    session_t* ss = find_session_by_id(req.argv[0]);
    if(ss == NULL){
        return FAIL;
    }
    ss->sse_fd = fd;

    // free(ss);
    ss = NULL;
    send_packet(fd, SSE_CONNECT, OK, "");

    return SUCCESS;
}

// 에코 
int cmd_echo(int fd, req_packet_t req_msg){
    printf("[%d] (echo) text : %s\n", fd, req_msg.argv[0]);
    res_packet_t res_msg = {
        .status = ECHO
    };
    snprintf(res_msg.chat_text, sizeof(res_msg.chat_text), "%s", req_msg.argv[0]);
    return send_to_client(fd, &res_msg);
}

// 채팅 메세지 전송 
int cmd_chat(int fd, req_packet_t req_msg){
    chat_type type = atoi(req_msg.argv[0]);
    int room_id = atoi(req_msg.argv[1]);
    char* text = req_msg.argv[2];
    int reply_id = atoi(req_msg.argv[3]);
    printf("[%d] (chat) %d-> text : %s\n", fd, room_id, text);

    session_t* ss = find_session_by_fd(fd);
    if(ss == NULL){
        perror("session");
        send_packet(fd, SEND_CHAT, Session_Not_Found, "Session Not Found");
        return FAIL;
    }

    char* user_id = ss->id;
    mysql_insert_chat(room_id, user_id, reply_id, type, text);

    if(send_chat_in_room(room_id) == FAIL){
        send_packet(fd, SEND_CHAT, ERROR, "send chat failed");
        ss = NULL;
        user_id = NULL;

        return FAIL;
    }
    ss = NULL;
    user_id = NULL;

    return SUCCESS;
}

// 채팅 공감 전송
int cmd_chat_react(int fd, req_packet_t req_msg){
    int room_id = atoi(req_msg.argv[0]);
    int chat_id = atoi(req_msg.argv[1]);
    int react_type = atoi(req_msg.argv[2]);
    printf("[%d] (chat react) %d-> chat_id:%d,react:%d\n", fd, room_id, chat_id, react_type);
    if(chat_id < 0){
        return FAIL;
    }

    session_t* ss = find_session_by_fd(fd);
    if(ss == NULL){
        perror("session");
        send_packet(fd, SEND_CHAT, Session_Not_Found, "Session Not Found");
        return FAIL;
    }

    char* user_id = ss->id;
    mysql_insert_chat_react(room_id, user_id, chat_id, react_type);

    if(send_chat_in_room(room_id) == FAIL){
        return FAIL;
    }

    return SUCCESS;
}

// 메세지 삭제 
int cmd_delete_chat(int fd, req_packet_t req_msg){
    int room_id = atoi(req_msg.argv[0]);
    int chat_id = atoi(req_msg.argv[1]);
    int flag = atoi(req_msg.argv[2]);
    printf("[%d] (chat delete) %d-> chat_id:%d,flag:%d\n", fd, room_id, chat_id, flag);


    session_t* ss = find_session_by_fd(fd);
    if(ss == NULL){
        perror("session");
        send_packet(fd, DELETE_CHAT, Session_Not_Found, "Session Not Found");
        return FAIL;
    }

    char* user_id = ss->id;
    if(mysql_delete_chat(room_id, user_id, chat_id, flag) == FAIL){
        send_packet(fd, DELETE_CHAT, ERROR, "mysql error");
         ss = NULL;
        user_id = NULL;
        return FAIL;
    }
    printf("send chat in room\n");
    if(send_chat_in_room(room_id) == FAIL){
        send_packet(fd, DELETE_CHAT, ERROR, "send chat failed");
        ss = NULL;
        user_id = NULL;

        return FAIL;
    }
    ss = NULL;
    user_id = NULL;
    return SUCCESS;
}


// 멤버 리스트 
int cmd_user_list(int fd, req_packet_t req_msg){
    printf("member_list\n");

    res_packet_t res_msg ={
        .status = USER_LIST,
        .msg = ""
    };

    int count = 0;

    return send_to_client(fd, &res_msg);
}

int cmd_create_room(int fd, req_packet_t req_msg){
    char* room_name = &req_msg.argv[0];
    printf("[%d] (create_room) room_name : %s\n",fd, room_name);

    int len = atoi(req_msg.argv[1]);

    char* p[16];
    for (int i = 0; i < len; ++i) {
        p[i] = req_msg.argv[i+2];
        // printf("p: %s\n", req_msg.argv[i+2]);
    }

    mysql_insert_room(room_name, p, len);

    send_packet(fd, CREATE_ROOM, OK, "");
    
    return SUCCESS;
}

int cmd_enter_room(int fd, req_packet_t req_msg){
    int room_num = atoi(req_msg.argv[0]);
    printf("[%d] (enter_room) room_num : %d\n",fd, room_num);
    return SUCCESS;
}

int cmd_exit_room(int fd, req_packet_t req_msg){
    printf("[%d] (exit_room) \n",fd);

    return SUCCESS;
}

int cmd_room_list(int fd, req_packet_t req_msg){
    printf("[%d] (room_list) \n",fd);

    session_t* ss = find_session_by_fd(fd);
    if(ss == NULL){
        send_packet(fd, Search_User_When_Create_Room, Session_Not_Found, "Session Not Found");
        return FAIL;
    }
    char* user_id = strdup(ss->id);

    mysql_bind_vo vo;

    res_packet_t res = {
        .header.type = UPDATE_ROOM_LIST,
        .status = OK
    };

    mysql_get_room_list(user_id, &res);
    // free(user_id);
    // free(ss);
    ss = NULL;

    send_to_client(fd, &res);
    return SUCCESS;
}

int cmd_search_user_when_create_room(int fd, req_packet_t req){
    printf("receive, search text : %s\n", req.argv[0]);
    print_session();

    session_t* ss = find_session_by_fd(fd);
    if(ss == NULL){
        send_packet(fd, Search_User_When_Create_Room, Session_Not_Found, "Session Not Found");
        return FAIL;
    }
    char* user_id = ss->id;

    mysql_bind_vo bind;

    if(mysql_search_user(user_id, req.argv[0], &bind) == FAIL){
        send_packet(fd, Search_User_When_Create_Room, ERROR, "mysql Error");
        return FAIL;
    }
    free(user_id);

    res_packet_t res = {
        .header.type = Search_User_When_Create_Room,
        .status = OK
    };
    memcpy(res.data, bind.data, sizeof(bind.data));
    res.data_len = bind.data_len;
    // free(ss);
    ss = NULL;

    return send_to_client(fd, &res);    
}

int cmd_update_chat_list(int fd, req_packet_t req){
    int room_id = atoi(req.argv[0]);
    printf("[%d] (update chat list) %d\n", fd, room_id);

    // print_session();
    session_t* ss = find_session_by_fd(fd);
    if(ss == NULL){
        send_packet(fd, UPDATE_CHAT_LIST, Session_Not_Found, "Session Not Found");
        return FAIL;
    }
    // printf("update chat list 1\n");
    char* user_id = ss->id;

    res_packet_t res = {
        .header.type = UPDATE_CHAT_LIST,
        .status = OK
    };
    // printf("update chat list 2\n");

    if(mysql_get_chat_list(room_id, user_id, &res) == FAIL){
        send_packet(fd, UPDATE_CHAT_LIST, MYSQL_ERROR, "(MYSQL) ERROR");
        return FAIL;
    }
    // free(ss);
    ss = NULL;

    return send_to_client(fd, &res); 
}