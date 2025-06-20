#ifndef MYSQL_COMMON_H
#define MYSQL_COMMON_H

#include "server_sock.h"
// #include <mysql/mysql.h>

typedef struct mysql_res_t{
    // char** rows;
    MYSQL_ROW* rows;
    int num_rows;
    int num_fields;
}mysql_res_t;

static int PRINT_QUERY_MODE = 0;

extern MYSQL *conn; //mysql과의 커넥션을 잡는데 지속적으로 사용되는 변수
extern MYSQL_RES *res;  //쿼리문에 대한 result값을 받는 주소값  
extern MYSQL_ROW row;   //쿼리문에 대한 실제 데이터값이 들어있는 변수   

int mysql_connect();

mysql_res_t* mysql_query_execute(
    const char* query,
    MYSQL_BIND* bind_params,
    int param_count
);

mysql_res_t* mysql_multi_query_execute(
    const char** queries,
    MYSQL_BIND** bind_sets,
    int* bind_counts,
    int query_count
);

void build_query_with_params(const char* query_template, MYSQL_BIND* params, int param_count);

#endif