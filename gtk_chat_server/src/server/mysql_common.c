#include "mysql_common.h"

MYSQL *conn = NULL; //mysql과의 커넥션을 잡는데 지속적으로 사용되는 변수에요.   
// MYSQL_RES *res;  //쿼리문에 대한 result값을 받는 위치변수에요.   
MYSQL_ROW row;   //쿼리문에 대한 실제 데이터값이 들어있는 변수에요. 

int mysql_connect(){
   conn = mysql_init(NULL);   
   if (conn == NULL) {
      perror("mysql_init");
      return FAIL;
   }
  
   if (!mysql_real_connect(conn, MYSQL_HOSTNAME,      
         MYSQL_USERNAME, MYSQL_PWD, MYSQL_DATABASE, 0, NULL, 0)) {   
      perror("mysql_real_connect");
      mysql_close(conn);
      conn = NULL;
      return FAIL;
   }
   printf("✅ MySQL connected successfully.\n");

   return SUCCESS;
}

mysql_res_t* mysql_query_execute(
    const char* query,
    MYSQL_BIND* bind_params,
    int param_count
) {
    // lock
    pthread_mutex_lock(&mysql_mutex);

    if (conn == NULL || mysql_ping(conn) != 0) {
        if (mysql_connect() == FAIL) {
            fprintf(stderr, "MySQL connection failed.\n");
            pthread_mutex_unlock(&mysql_mutex);

            return NULL;
        }
    }

    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        fprintf(stderr, "mysql_stmt_init() failed\n");
        pthread_mutex_unlock(&mysql_mutex);

        return NULL;
    }

    if (mysql_stmt_prepare(stmt, query, (unsigned long)strlen(query)) != 0) {
        fprintf(stderr, "mysql_stmt_prepare() failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        pthread_mutex_unlock(&mysql_mutex);
        return NULL;
    }

    if (param_count > 0 && bind_params) {
        if (mysql_stmt_bind_param(stmt, bind_params) != 0) {
            fprintf(stderr, "mysql_stmt_bind_param() failed: %s\n", mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            pthread_mutex_unlock(&mysql_mutex);

            return NULL;
        }
    }
    build_query_with_params(query, bind_params, param_count);
    if(stmt == NULL){
        printf("stmt is NULL\n");
        return NULL;
    }

    if (mysql_stmt_execute(stmt) != 0) {
        perror("mysql_stmt_execute");
        fprintf(stderr, "mysql_stmt_execute() failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        pthread_mutex_unlock(&mysql_mutex);

        return NULL;
    }

    MYSQL_RES* meta = mysql_stmt_result_metadata(stmt);
    if (!meta) {
        mysql_stmt_close(stmt);
        pthread_mutex_unlock(&mysql_mutex);

        return NULL;
    }

    int num_fields = mysql_num_fields(meta);
    MYSQL_FIELD* fields = mysql_fetch_fields(meta);

    MYSQL_BIND*    result_bind   = calloc(num_fields, sizeof(MYSQL_BIND));
    char**         field_buffers = calloc(num_fields, sizeof(char*));
    unsigned long* lengths       = calloc(num_fields, sizeof(unsigned long));

    for (int i = 0; i < num_fields; i++) {
        field_buffers[i]             = calloc(256, 1);
        result_bind[i].buffer_type   = MYSQL_TYPE_STRING;
        result_bind[i].buffer        = field_buffers[i];
        result_bind[i].buffer_length = 256;
        result_bind[i].length        = &lengths[i];
    }
    mysql_stmt_bind_result(stmt, result_bind);

    mysql_res_t* result = malloc(sizeof(mysql_res_t));
    result->num_fields = num_fields;
    result->num_rows   = 0;
    result->rows       = malloc(sizeof(MYSQL_ROW) * 100);

    printf("--- QUERY RESULT ---\n|");
    for (int i = 0; i < num_fields; i++) {
        printf(" %-20s |", fields[i].name);
    }
    printf("\n");

    int fetch_status;
    while ((fetch_status = mysql_stmt_fetch(stmt)) == 0 ||
           fetch_status == MYSQL_DATA_TRUNCATED) {
        char** row = malloc(sizeof(char*) * num_fields);
        printf("|");
        for (int i = 0; i < num_fields; i++) {
            const char* val = (field_buffers[i][0] != '\0')
                                ? field_buffers[i]
                                : "NULL";
            row[i] = strdup(val);
            printf(" %-20s |", val);
        }
        printf("\n");
        result->rows[result->num_rows++] = row;
        /* clear buffers so NULL columns don't reuse old data */
        for (int i = 0; i < num_fields; i++) {
            field_buffers[i][0] = '\0';
        }
    }
    printf("--- QUERY END (rows: %d) ---\n\n\n", result->num_rows);

    mysql_free_result(meta);
    mysql_stmt_close(stmt);
    for (int i = 0; i < num_fields; i++) {
        free(field_buffers[i]);
    }
    free(field_buffers);
    free(result_bind);
    free(lengths);

    pthread_mutex_unlock(&mysql_mutex);

    return result;
}

mysql_res_t* mysql_multi_query_execute(
    const char** queries,
    MYSQL_BIND** bind_sets,
    int* bind_counts,
    int query_count
) {
    pthread_mutex_lock(&mysql_mutex);
    // printf("[T%5d] LOCK   conn=%p\n", mysql_thread_id(conn), conn);

    if(conn == NULL){
        mysql_connect();
    }
    MYSQL_RES* final_meta = NULL;
    MYSQL_STMT* stmt = NULL;
    MYSQL_BIND* result_bind = NULL;
    mysql_res_t* result = NULL;

    for (int i = 0; i < query_count; i++) {
        // printf("q [%d]\n", i);
        if(conn == NULL){
            printf("conn is null [%d]\n", i);
            mysql_connect();
        }

        stmt = mysql_stmt_init(conn);
        if (!stmt) {
            perror("mysql_stmt_init");
            // printf("[T%5d] UNLOCK conn=%p\n", mysql_thread_id(conn), conn);
            pthread_mutex_unlock(&mysql_mutex);

            return NULL;
        }
        // printf("q 33 [%d]\n", i);

        if(queries[i] == NULL){
            printf("quries[%d] is NULL\n", i);
            continue;
        }
        int q_len = strlen(queries[i]) ;

        // printf("quries[%d] size is %d\n", strlen(queries[i]));
        if(q_len <= 0){
            continue;
        }

        if (mysql_stmt_prepare(stmt, queries[i], strlen(queries[i])) != 0) {
            fprintf(stderr, "prepare failed (%d): %s\n", i, mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            // printf("[T%5d] UNLOCK conn=%p\n", mysql_thread_id(conn), conn);
            pthread_mutex_unlock(&mysql_mutex);

            return NULL;
        }

        if (bind_counts[i] > 0 && bind_sets[i]) {
            if (mysql_stmt_bind_param(stmt, bind_sets[i]) != 0) {
                fprintf(stderr, "bind_param failed (%d): %s\n", i, mysql_stmt_error(stmt));
                mysql_stmt_close(stmt);
                // printf("[T%5d] UNLOCK conn=%p\n", mysql_thread_id(conn), conn);
    pthread_mutex_unlock(&mysql_mutex);

                return NULL;
            }
        }

        build_query_with_params(queries[i], bind_sets[i], bind_counts[i]);

        if (mysql_stmt_execute(stmt) != 0) {
            fprintf(stderr, "execute failed (%d): %s\n", i, mysql_stmt_error(stmt));
            mysql_stmt_close(stmt);
            // printf("[T%5d] UNLOCK conn=%p\n", mysql_thread_id(conn), conn);
    pthread_mutex_unlock(&mysql_mutex);

            return NULL;
        }

        // 마지막 SELECT 쿼리만 결과 추출
        if (i == query_count - 1) {
            final_meta = mysql_stmt_result_metadata(stmt);
            if (!final_meta) {
                mysql_stmt_close(stmt);
                // printf("[T%5d] UNLOCK conn=%p\n", mysql_thread_id(conn), conn);
    pthread_mutex_unlock(&mysql_mutex);

                return NULL;
            }

            int num_fields = mysql_num_fields(final_meta);
            MYSQL_BIND* result_bind = calloc(num_fields, sizeof(MYSQL_BIND));
            char** field_buffers = calloc(num_fields, sizeof(char*));
            unsigned long* lengths = calloc(num_fields, sizeof(unsigned long));

            for (int j = 0; j < num_fields; j++) {
                field_buffers[j] = calloc(256, sizeof(char));
                result_bind[j].buffer_type = MYSQL_TYPE_STRING;
                result_bind[j].buffer = field_buffers[j];
                result_bind[j].buffer_length = 256;
                result_bind[j].length = &lengths[j];
            }

            mysql_stmt_bind_result(stmt, result_bind);

            MYSQL_FIELD* fields = mysql_fetch_fields(final_meta);


            // fetch rows
            result = malloc(sizeof(mysql_res_t));
            result->num_fields = num_fields;
            result->num_rows = 0;
            result->rows = malloc(sizeof(char**) * 100); // 최대 100개 가정

            printf("--- QUERY RESULT ---\n");
            printf("|");
            for (int i = 0; i < num_fields; i++) {
                printf(" %-20s |", fields[i].name);
            }
            printf("|");
            while (mysql_stmt_fetch(stmt) == 0) {
                char** row = malloc(sizeof(char*) * num_fields);
                printf("|");
                for (int j = 0; j < num_fields; j++) {
                    printf("%10s|", field_buffers[j]);
                    row[j] = strdup(field_buffers[j]);
                }
                printf("\n");
                result->rows[result->num_rows++] = row;
            }
            printf("--- QUERY END (rows: %d) ---\n\n\n", result->num_rows);


            // cleanup
            for (int j = 0; j < num_fields; j++) {
                free(field_buffers[j]);
            }
            free(field_buffers);
            free(result_bind);
            free(lengths);
        }

        mysql_stmt_close(stmt);
    }

    pthread_mutex_unlock(&mysql_mutex);
    // printf("[T%5d] UNLOCK conn=%p\n", mysql_thread_id(conn), conn);
    return result;
}

// GPT 코드 


void build_query_with_params(const char* query_template, MYSQL_BIND* params, int param_count) {
    if(PRINT_QUERY_MODE != 0){
        return;
    }
    char* final_query = malloc(4096);
    if (!final_query) return;
    final_query[0] = '\0'; // 초기화

    const char* p = query_template;
    int param_index = 0;
    char temp[256];

    while (*p && param_index < param_count) {
        if (*p == '?') {
            // 바인딩된 파라미터를 문자열로 변환
            switch (params[param_index].buffer_type) {
                case MYSQL_TYPE_STRING:
                    snprintf(temp, sizeof(temp), "'%s'", (char*)params[param_index].buffer);
                    break;
                case MYSQL_TYPE_LONG:
                    snprintf(temp, sizeof(temp), "%d", *(int*)params[param_index].buffer);
                    break;
                case MYSQL_TYPE_LONGLONG:
                    snprintf(temp, sizeof(temp), "%lld", *(long long*)params[param_index].buffer);
                    break;
                case MYSQL_TYPE_DOUBLE:
                    snprintf(temp, sizeof(temp), "%f", *(double*)params[param_index].buffer);
                    break;
                default:
                    snprintf(temp, sizeof(temp), "'(unsupported)'");
            }

            strcat(final_query, temp);
            param_index++;
            p++; // ? 다음 문자로 이동
        } else {
            strncat(final_query, p, 1); // 일반 문자 복사
            p++;
        }
    }

    // 나머지 템플릿 문자열 복사
    printf("QUERY: %s\n", final_query);
}
