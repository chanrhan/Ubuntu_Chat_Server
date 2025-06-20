#ifndef JWT_COMMON_H
#define JWT_COMMON_H

#include "common.h"
#include <jwt.h>

// 실제 서비스한다면 환경변수에 저장해놓는 것이 안전
// 과제 프로젝트기 때문에 하드코딩으로 선언
#define JWT_SECRET_KEY "8ca50554f0af7f04e5e2f61605bf96328158d40b736aed883e33c77d3a28e9fb"

long create_jwt(char* id, int expired_time, char** out);

int validate_jwt(char* token);


#endif