#include "common.h"



int is_empty_string(char* str){
    return (str == NULL || strcmp(str, "") == 0) ? SUCCESS : FAIL;
}

char* concat_strings(int count, ...) {
    va_list args;
    va_start(args, count);

    // 총 길이 계산
    size_t total_len = 0;
    for (int i = 0; i < count; i++) {
        const char* str = va_arg(args, const char*);
        total_len += strlen(str);
    }
    va_end(args);

    // 메모리 할당
    char* result = malloc(total_len + 1); // +1 for '\0'
    if (!result) return NULL;
    result[0] = '\0'; // 초기화

    // 문자열 붙이기
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        const char* str = va_arg(args, const char*);
        strcat(result, str);
    }
    va_end(args);

    return result;
}

char* append_string(char* original, const char* new_str) {
    size_t new_len = strlen(original) + strlen(new_str) + 1;
    char* result = realloc(original, new_len);
    if (!result) {
        fprintf(stderr, "[append_string] realloc failed\n");
        exit(1); // 또는 NULL 리턴
    }
    strcat(result, new_str);
    return result;
}