#ifndef MYSQL_QUERY_H
#define MYSQL_QUERY_H

#include "mysql_common.h"

int mysql_refresh_token(char* id, char* token, long exp_tm);

int mysql_exist_refresh_token(char* token, long exp_tm, char** out_id);
int mysql_exist_id(char* id);
int mysql_insert_user(user_vo* vo); //signup

int mysql_match_user(user_vo* vo); // login

int mysql_search_user(char* id, char* search_text, mysql_bind_vo* bind);

int mysql_insert_room(char* room_name, char** init_users, int len);

int mysql_get_room_list(char* id, res_packet_t* bind_vo);

int mysql_get_chat_list(int room_id, char* id, res_packet_t* out_res);

int mysql_insert_chat(int room_id, char* id, int reply_id, chat_type type, char* text);

int mysql_get_inner_room_members(int room_id, chat_vo** out);

int mysql_insert_chat_react(int room_id, char* user_id, int chat_id, chat_react_type type);
int mysql_delete_chat(int room_id, char* user_id, int chat_id, int flag);

#endif