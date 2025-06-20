#include "mysql_query.h"

int create_bind_param(MYSQL_BIND* param, enum_field_types type, void* buffer, unsigned long length) {
    memset(param, 0, sizeof(MYSQL_BIND));  

    param->buffer_type = type;             
    param->buffer = buffer;                
    param->buffer_length = length;         
    param->is_null = 0;                    
    param->length = malloc(sizeof(unsigned long));
    *param->length = length+1;

    return 0; // 성공
}

int mysql_refresh_token(char* id, char* token, long exp_tm){
    const char* query = "select count(*) from refresh_tok where user_id=? ";

    MYSQL_BIND bind[1];
    create_bind_param(&bind[0], MYSQL_TYPE_STRING, id, strlen(id));

    mysql_res_t* res = mysql_query_execute(query, bind, 1);
    if(res == NULL || res->num_rows <= 0){
        return FAIL;
    }

    if(strcmp(res->rows[0][0], "1") == 0){
        printf("update\n");
        // update
        const char* query2 = "update refresh_tok set token=?, revoked=false, exp_tm=? where user_id=?";
        
        MYSQL_BIND bind2[3];
        create_bind_param(&bind2[0], MYSQL_TYPE_STRING, token, strlen(token));
        create_bind_param(&bind2[1], MYSQL_TYPE_LONG, &exp_tm, sizeof(exp_tm));
        create_bind_param(&bind2[2], MYSQL_TYPE_STRING, id, strlen(id));

        mysql_res_t* res2 = mysql_query_execute(query2, bind2, 3);
        // if(res2 == NULL || res2->num_rows <= 0){
        //     return SUCCESS;
        // }
        return SUCCESS;

    }else if(strcmp(res->rows[0][0], "0") == 0){
        printf("insert\n");

        // insert
        const char* query3 = "insert into refresh_tok (user_id,token,exp_tm) values (?,?,?)";
        
        MYSQL_BIND bind3[3];
        create_bind_param(&bind3[0], MYSQL_TYPE_STRING, id, strlen(id));
        create_bind_param(&bind3[1], MYSQL_TYPE_STRING, token, strlen(token));
        create_bind_param(&bind3[2], MYSQL_TYPE_LONG, &exp_tm, sizeof(exp_tm));

        mysql_res_t* res2 = mysql_query_execute(query3, bind3, 3);
        // printf("res : %d\n", res2->num_rows);
        // if(res2 == NULL || res2->num_rows <= 0){
        //     return SUCCESS;
        // }
        return SUCCESS;
    }
    return FAIL;
}

int mysql_exist_refresh_token(char* token, long exp_tm, char** out_id){
    const char* query = "select user_id, revoked, exp_tm from refresh_tok where token=? ";

    MYSQL_BIND bind[1];
    create_bind_param(&bind[0], MYSQL_TYPE_STRING, token, strlen(token));

    mysql_res_t* res = mysql_query_execute(query, bind, 1);
    if(res == NULL || res->num_rows <= 0){
        return FAIL;
    }

    int revoked = atoi(res->rows[0][1]);
    if(revoked == 0){
        printf("refresh token is revoked!\n");
        return FAIL;
    }
    if(exp_tm != atol(res->rows[0][2])){
        printf("Invalid expired time!\n");
        return FAIL;
    }

    *out_id = res->rows[0][0];
    
    return SUCCESS;
}


int mysql_exist_id(char* id){
    const char* query = "select * from user where id=?";
    MYSQL_BIND bind[1];
    create_bind_param(&bind[0], MYSQL_TYPE_STRING, id, strlen(id));

    mysql_res_t* res = mysql_query_execute(query, bind, 1);
    if(res == NULL){
        return SUCCESS;
    }


    return res->num_rows == 0 ? SUCCESS : FAIL;
}

int mysql_insert_user(user_vo* vo){
    const char* query = "insert into user (id, pwd, name) values (?,?,?)";
    MYSQL_BIND bind[3];
    create_bind_param(&bind[0], MYSQL_TYPE_STRING, vo->id, 16);
    create_bind_param(&bind[1], MYSQL_TYPE_STRING, vo->pwd, 128);
    create_bind_param(&bind[2], MYSQL_TYPE_STRING, vo->name, 16);

    mysql_query_execute(query, bind, 3);
    // printf("res row:%d, col:%d\n", res->num_rows, res->num_fields);


    return 1;
}

int mysql_match_user(user_vo* vo){

    const char* query = "select count(*) from user where id=? and pwd=?";
    MYSQL_BIND bind[2];
    create_bind_param(&bind[0], MYSQL_TYPE_STRING, vo->id, 16);
    create_bind_param(&bind[1], MYSQL_TYPE_STRING, vo->pwd, 128);

    mysql_res_t* res = mysql_query_execute(query, bind, 2);
    if (res == NULL || res->num_rows == 0 || res->rows[0] == NULL || res->rows[0][0] == NULL) {
        perror("[MYSQL] result is NULL");
        return FAIL;
    }
    // printf("num_rows: %d\n", res->num_rows);
    // printf("res: %s\n", res->rows[0][0]);

    return (strcmp(res->rows[0][0], "1") == 0) ? SUCCESS : FAIL;
}

int mysql_search_user(char* id, char* search_text, mysql_bind_vo* bind_vo){
    const char* query = "select id,name from user where id !=? and name like ? limit 5";

    char like_param[64];
    snprintf(like_param, sizeof(like_param), "%%%s%%", search_text);

    MYSQL_BIND bind[2];
    create_bind_param(&bind[0], MYSQL_TYPE_STRING, id, strlen(id));
    create_bind_param(&bind[1], MYSQL_TYPE_STRING, like_param, strlen(like_param));


    mysql_res_t* res = mysql_query_execute(query, bind, 2);
    if (res == NULL || res->num_rows == 0 || res->rows[0] == NULL || res->rows[0][0] == NULL) {
        perror("[MYSQL] result is NULL");
        return SUCCESS;
    }

    user_vo* bind_results = (user_vo*)bind_vo->data;

    for(int r=0;r<res->num_rows;++r){
        // for(int c=0;c<res->num_fields;++c){ 
        //     printf("%s\n", res->rows[r][c]);
        // }
        // printf("\n");
        snprintf(bind_results[r].id, 16, "%s", res->rows[r][0]);
        snprintf(bind_results[r].name, 16, "%s", res->rows[r][1]);
    }
    bind_vo->data_len = res->num_rows;
    return SUCCESS;
}

int mysql_insert_room(char* room_name, char** init_users, int len){
    const char* query[] = {
        "set @max_id := (select IFNULL(max(room_id)+1, 0) from room); ",
        "insert into room (room_id, room_name) values (@max_id, ?);",
        "select @max_id;"
    } ;

    MYSQL_BIND bind[1];
    create_bind_param(&bind[0], MYSQL_TYPE_STRING, room_name, strlen(room_name));

    MYSQL_BIND** bind_set = (MYSQL_BIND*[]){
        NULL, bind, NULL
    };
    int bind_counts[] = {0,1,0};
    
    mysql_res_t* res = mysql_multi_query_execute(query, bind_set, bind_counts, 3);
    if (res == NULL || res->num_rows == 0 || res->rows[0] == NULL || res->rows[0][0] == NULL) {
        perror("[MYSQL] result is NULL");
        return FAIL;
    }

    int room_id = atoi(res->rows[0][0]);

    char* query2 = strdup("insert into room_member (user_id, room_id) values ");
    MYSQL_BIND bind2[len];

    for (int k = 0; k < len; ++k) {
        char nq[32];
        snprintf(nq, sizeof(nq), "(?, %d)%s", room_id, (k < len - 1 ? "," : ""));

        char* new_query = append_string(query2, nq);
        if (!new_query) {
            fprintf(stderr, "append_string failed\n");
            free(query2);
            return FAIL;
        }
        query2 = new_query;

        // printf("qq: %s\n", query2);
        create_bind_param(&bind2[k], MYSQL_TYPE_STRING, init_users[k], strlen(init_users[k]));
    }
    
    mysql_res_t* res2 = mysql_query_execute(query2, bind2, len);
    free(query2);
    return SUCCESS;
    // if (res2 == NULL || res2->num_rows == 0 || res2->rows[0] == NULL || res2->rows[0][0] == NULL) {
    //     perror("[MYSQL] result is NULL");
    //     return FAIL;
    // }

    // printf("res insert: %d\n", atoi(res2->rows[0][0]));
    // return 1;
}

/*
select rm.room_id, r.room_name, (cm.max_chat_id - rm.last_read) as updated_chat_count, c2.text as last_chat, c2.send_dt as last_chat_time from room_member rm left outer join room r on r.room_id=rm.room_id left outer join ( select c.room_id, IFNULL(max(c.chat_id), 0) as max_chat_id from chat c  group by c.room_id ) cm on cm.room_id=rm.room_id left outer join chat c2 on c2.room_id=cm.room_id and c2.chat_id=cm.max_chat_id where rm.user_id='aaa';

*/

int mysql_get_room_list(char* id, res_packet_t* out_res){
    const char* query = 
    "SELECT "
    "rm.room_id, "
    "r.room_name, "
    "(cm.max_chat_id - rm.last_read) AS updated_chat_count, "
    "c2.text AS last_chat, "
    "c2.send_dt AS last_chat_time, "
    "( select IFNULL(count(*), 0) from room_member rm2 where rm2.room_id=rm.room_id) as member_num "
    "FROM room_member rm "
    "LEFT outer JOIN room r "
    "ON r.room_id = rm.room_id "
    "LEFT outer JOIN ( "
    "SELECT " 
    "    c.room_id, "
    "    IFNULL(MAX(c.chat_id),0) AS max_chat_id "
    "FROM chat c "
    "GROUP BY c.room_id "
    ") cm "
    "ON cm.room_id = rm.room_id "
    "LEFT outer JOIN chat c2 "
    "ON c2.room_id = cm.room_id "
    "AND c2.chat_id = cm.max_chat_id "
    "WHERE rm.user_id=?";

    MYSQL_BIND bind[1];
    create_bind_param(&bind[0], MYSQL_TYPE_STRING, id, strlen(id));

    mysql_res_t* res = mysql_query_execute(query, bind, 1);
    if (res == NULL || res->num_rows == 0 || res->rows[0] == NULL || res->rows[0][0] == NULL) {
        perror("[MYSQL] result is NULL");
        return SUCCESS;
    }
    if(sizeof(room_vo) * res->num_rows >= MAX_PACKET_SIZE){
        printf("[MYSQL] mysql_get_room_list() : Too many rows! %d (max: %d)\n", sizeof(room_vo)*res->num_rows, MAX_PACKET_SIZE);
        return FAIL;
    }
    room_vo* bind_results = (room_vo*)out_res->data;

    for(int r=0;r<res->num_rows;++r){
        if (!res->rows[r]) {
            printf("[ERROR] res->rows[%d] is NULL\n", r);
            continue;
        }
        bind_results[r].room_id = atoi(res->rows[r][0]);
        // printf("mysql r id : %d\n", bind_results[r].room_id);
        snprintf(bind_results[r].room_name, sizeof(bind_results[r].room_name), "%s", res->rows[r][1] ? res->rows[r][1] : "");
        bind_results[r].updated_chat_count = atoi(res->rows[r][2]);
        snprintf(bind_results[r].last_chat, sizeof(bind_results[r].last_chat), "%s", res->rows[r][3] ? res->rows[r][3] : "");
        snprintf(bind_results[r].last_chat_time, sizeof(bind_results[r].last_chat_time), "%s", res->rows[r][4] ? res->rows[r][4] : "");
        bind_results[r].member_num = atoi(res->rows[r][5]);
    }
    out_res->data_len = res->num_rows;
    return SUCCESS;
}


int mysql_get_chat_list(int room_id, char* user_id, res_packet_t* out_res){
    const char *query[] = {
        "update room_member rm "
        "set last_read=(select IFNULL(max(c.chat_id)+1, 0) from chat c where c.room_id=?) "
        "where rm.room_id=? and rm.user_id=? ",

        "select * from ( select c.chat_id, "
        "c.chat_type, "
        "u.id as owner_id, "
        "u.name as owner_name, "
        "c.text, "
        "c.reply_chat_id, "
        "(select IFNULL(count(*), 0) from room_member rm where rm.room_id=c.room_id and rm.last_read <= c.chat_id) as non_read_count, "
        "c.send_dt, "
        "(select IF(c.deleted=1 or (select count(*) from chat_deleted cd where cd.room_id=c.room_id and cd.user_id=? and cd.chat_id=c.chat_id)=1, 1,0)) as deleted, "
        "(select count(*) from chat_react cr where cr.room_id=c.room_id and cr.chat_id=c.chat_id and cr.react_type=0) as emo_love_cnt, "
        "(select count(*) from chat_react cr where cr.room_id=c.room_id and cr.chat_id=c.chat_id and cr.react_type=1) as emo_like_cnt, "
        "(select count(*) from chat_react cr where cr.room_id=c.room_id and cr.chat_id=c.chat_id and cr.react_type=2) as emo_check_cnt, "
        "(select count(*) from chat_react cr where cr.room_id=c.room_id and cr.chat_id=c.chat_id and cr.react_type=3) as emo_laugh_cnt, "
        "(select count(*) from chat_react cr where cr.room_id=c.room_id and cr.chat_id=c.chat_id and cr.react_type=4) as emo_surprise_cnt, "
        "(select count(*) from chat_react cr where cr.room_id=c.room_id and cr.chat_id=c.chat_id and cr.react_type=5) as emo_sad_cnt "
        "from chat c "
        "left outer join user u on c.user_id=u.id "
        "where c.room_id=? "
        "order by c.send_dt desc "
        "limit 50 ) tmp "
        "order by send_dt asc "
    };

     MYSQL_BIND bind1[3];
    create_bind_param(&bind1[0], MYSQL_TYPE_LONG, &room_id, sizeof(room_id));
    create_bind_param(&bind1[1], MYSQL_TYPE_LONG, &room_id, sizeof(room_id));
    create_bind_param(&bind1[2], MYSQL_TYPE_STRING, user_id, strlen(user_id));

    MYSQL_BIND bind2[2];
    create_bind_param(&bind2[0], MYSQL_TYPE_STRING, user_id, strlen(user_id));
    create_bind_param(&bind2[1], MYSQL_TYPE_LONG, &room_id, sizeof(room_id));

   
    MYSQL_BIND** bind_set = (MYSQL_BIND*[]){
        bind1, bind2
    };
    int bind_counts[] = {3,2};
    

    mysql_res_t* res = mysql_multi_query_execute(query, bind_set, bind_counts , 2);

    if (res == NULL || res->num_rows == 0 || res->rows[0] == NULL || res->rows[0][0] == NULL) {
        perror("[MYSQL] result is NULL");
        return SUCCESS;
    }

    if(sizeof(chat_vo) * res->num_rows >= MAX_PACKET_SIZE){
        printf("[MYSQL] mysql_select_chat_list() : Too many rows! %d (max: %d)\n", sizeof(chat_vo)*res->num_rows, MAX_PACKET_SIZE);
        return FAIL;
    }

    chat_vo* bind_results = (chat_vo*)out_res->data;

    for(int r=0;r<res->num_rows;++r){
        if (!res->rows[r]) {
            printf("[ERROR] res->rows[%d] is NULL\n", r);
            continue;
        }

        bind_results[r].chat_id = atoi(res->rows[r][0]);
        bind_results[r].chat_type = atoi(res->rows[r][1]);
        snprintf(bind_results[r].owner_id, sizeof(bind_results[r].owner_id), "%s", res->rows[r][2]);
        snprintf(bind_results[r].owner_name, sizeof(bind_results[r].owner_name), "%s", res->rows[r][3]);
        snprintf(bind_results[r].text, sizeof(bind_results[r].text), "%s", res->rows[r][4]);
        bind_results[r].reply_chat_id = atoi(res->rows[r][5]);
        bind_results[r].non_read_count = atoi(res->rows[r][6]);
        snprintf(bind_results[r].chat_time, sizeof(bind_results[r].chat_time), "%s", res->rows[r][7]);
        bind_results[r].deleted = atoi(res->rows[r][8]);

        int total_cnt = 0;
        for(int k=0;k<6;++k){
            bind_results[r].emo_list[k] = atoi(res->rows[r][9+k]);
            total_cnt += bind_results[r].emo_list[k];
        }
        bind_results[r].emo_total_cnt = total_cnt;
                                            
        // printf("%s|%s|%d|%s\n", bind_results[r].owner_name, bind_results[r].text, bind_results[r].non_read_count, bind_results[r].time_text);
    }

    out_res->data_len = res->num_rows;
    return SUCCESS;
}

int mysql_insert_chat(int room_id, char* id, int reply_id, chat_type type, char* text){
    // printf("insert chat: %d,%s : %s\n", room_id, id, text);
    const char* query[] = {
        "set @max_id := (select IFNULL(max(chat_id)+1, 0) from chat where room_id=? ); ",
        "insert into chat (room_id, user_id, chat_id, reply_chat_id, chat_type, text) values (?,?,@max_id,?,?,?);"
    } ;

    MYSQL_BIND bind1[1];
    create_bind_param(&bind1[0], MYSQL_TYPE_LONG, &room_id, sizeof(room_id));

    MYSQL_BIND bind2[5];
    create_bind_param(&bind2[0], MYSQL_TYPE_LONG, &room_id, sizeof(room_id));
    create_bind_param(&bind2[1], MYSQL_TYPE_STRING, id, strlen(id));
    create_bind_param(&bind2[2], MYSQL_TYPE_LONG, &reply_id, sizeof(reply_id));
    create_bind_param(&bind2[3], MYSQL_TYPE_LONG, &type, sizeof(type));
    create_bind_param(&bind2[4], MYSQL_TYPE_STRING, text, strlen(text));

    MYSQL_BIND** bind_set = (MYSQL_BIND*[]){
        bind1, bind2
    };
    int bind_counts[] = {1,5};
    
    mysql_res_t* res = mysql_multi_query_execute(query, bind_set, bind_counts, 2);
    if (res == NULL || res->num_rows == 0 || res->rows[0] == NULL || res->rows[0][0] == NULL) {
        perror("[MYSQL] result is NULL");
        return FAIL;
    }

    return SUCCESS;
}

// 
int mysql_get_inner_room_members(int room_id, chat_vo** out){
     const char *query[] = {
        "set @max_chat_id := (select IFNULL(max(chat_id), 0) from chat where room_id=?) ",

        "select user_id, "
        "(select text from chat c1 where c1.room_id=? and c1.chat_id=@max_chat_id) as last_chat, "
        "(select send_dt from chat c2 where c2.room_id=? and c2.chat_id=@max_chat_id) as chat_time, "
        "(@max_chat_id - rm.last_read) as non_read_count "
        "from room_member rm "
        "where rm.room_id=?"
    };

    MYSQL_BIND bind1[1];
    create_bind_param(&bind1[0], MYSQL_TYPE_LONG, &room_id, sizeof(room_id));

    MYSQL_BIND bind2[4];
    create_bind_param(&bind2[0], MYSQL_TYPE_LONG, &room_id, sizeof(room_id));
    create_bind_param(&bind2[1], MYSQL_TYPE_LONG, &room_id, sizeof(room_id));
    create_bind_param(&bind2[2], MYSQL_TYPE_LONG, &room_id, sizeof(room_id));
    create_bind_param(&bind2[3], MYSQL_TYPE_LONG, &room_id, sizeof(room_id));

   
    MYSQL_BIND** bind_set = (MYSQL_BIND*[]){
        bind1, bind2
    };
    int bind_counts[] = {1,4};

    mysql_res_t* res = mysql_multi_query_execute(query, bind_set, bind_counts , 2);
    if (res == NULL || res->num_rows == 0 || res->rows[0] == NULL || res->rows[0][0] == NULL) {
        perror("[MYSQL] result is NULL");
        return SUCCESS;
    }

    *out = malloc(sizeof(chat_vo) * res->num_rows);
    if (!*out) return FAIL;
    for(int r=0;r<res->num_rows;++r){
    // printf("[%d]\n", r);

        if (!res->rows[r]) {
            printf("[ERROR] res->rows[%d] is NULL\n", r);
            continue;
        }
        snprintf((*out)[r].owner_id, 16, "%s", res->rows[r][0]);
        snprintf((*out)[r].text, 64, "%s", res->rows[r][1]);
        snprintf((*out)[r].chat_time, 32, "%s", res->rows[r][2]);
        (*out)[r].non_read_count = atoi(res->rows[r][3]);
        // (*out)[r].reply_chat_id = atoi(res->rows[r][4]);
    }

    return res->num_rows;
}

int mysql_insert_chat_react(int room_id, char* user_id, int chat_id, chat_react_type type){
    const char* query = "select * from chat_react where room_id=? and user_id=? and chat_id=?";

    MYSQL_BIND bind[3];
    create_bind_param(&bind[0], MYSQL_TYPE_LONG, &room_id, sizeof(room_id));
    create_bind_param(&bind[1], MYSQL_TYPE_STRING, user_id, strlen(user_id));
    create_bind_param(&bind[2], MYSQL_TYPE_LONG, &chat_id, sizeof(chat_id));


    mysql_res_t* res = mysql_query_execute(query, bind, 3);
    if (res == NULL) {
        perror("[MYSQL] result is NULL");
        return FAIL;
    }

    if(res->num_rows == 0){
        const char* query2 = "insert into chat_react (room_id,user_id,chat_id,react_type) values (?,?,?,?)";

        MYSQL_BIND bind2[4];
        create_bind_param(&bind2[0], MYSQL_TYPE_LONG, &room_id, sizeof(room_id));
        create_bind_param(&bind2[1], MYSQL_TYPE_STRING, user_id, strlen(user_id));
        create_bind_param(&bind2[2], MYSQL_TYPE_LONG, &chat_id, sizeof(chat_id));
        create_bind_param(&bind2[3], MYSQL_TYPE_LONG, &type, sizeof(type));


        mysql_res_t* res2 = mysql_query_execute(query2, bind2, 4);
        return SUCCESS;
    }else if(res->num_rows == 1){
        const char* query2 = "update chat_react set react_type=? where room_id=? and user_id=? and chat_id=?";

        MYSQL_BIND bind2[4];
        create_bind_param(&bind2[0], MYSQL_TYPE_LONG, &type, sizeof(type));
        create_bind_param(&bind2[1], MYSQL_TYPE_LONG, &room_id, sizeof(room_id));
        create_bind_param(&bind2[2], MYSQL_TYPE_STRING, user_id, strlen(user_id));
        create_bind_param(&bind2[3], MYSQL_TYPE_LONG, &chat_id, sizeof(chat_id));

        mysql_res_t* res2 = mysql_query_execute(query2, bind2, 4);
        if(res2 == NULL || res->num_rows <= 0){
            perror("[MYSQL] result is NULL");
            return FAIL;
        }
        return SUCCESS;
    }else{
        perror("[MYSQL] chat react Result too many!");
        return FAIL;
    }

    return SUCCESS;
}

int mysql_delete_chat(int room_id, char* user_id, int chat_id, int flag){
    if(flag == 1){
        // delete all
        printf("delete all\n");
        const char* query = "update chat set deleted=1 where room_id=? and chat_id=?";

        MYSQL_BIND bind[2];
        create_bind_param(&bind[0], MYSQL_TYPE_LONG, &room_id, sizeof(room_id));
        create_bind_param(&bind[1], MYSQL_TYPE_LONG, &chat_id, sizeof(chat_id));

        mysql_res_t* res = mysql_query_execute(query, bind, 2);
        // if (res == NULL || res->num_rows <= 0) {
        //     perror("[MYSQL] result is NULL");
        //     return FAIL;
        // }
    }else{
        // delete self 
        printf("delete self\n");

        const char* query = "insert into chat_deleted (room_id,user_id,chat_id) values (?,?,?)";

        MYSQL_BIND bind[3];
        create_bind_param(&bind[0], MYSQL_TYPE_LONG, &room_id, sizeof(room_id));
        create_bind_param(&bind[1], MYSQL_TYPE_STRING, user_id, strlen(user_id));
        create_bind_param(&bind[2], MYSQL_TYPE_LONG, &chat_id, sizeof(chat_id));

        mysql_res_t* res = mysql_query_execute(query, bind, 3);
        // if (res == NULL) {
        //     perror("[MYSQL] result is NULL");
        //     return FAIL;
        // }
    }
    return SUCCESS;
}