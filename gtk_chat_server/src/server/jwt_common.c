#include "jwt_common.h"


long create_jwt(char* id, int expired_time, char** out){
    jwt_t *jwt = NULL;
    char* encoded = NULL;

    if (jwt_new(&jwt) != 0) {
        fprintf(stderr, "jwt_new() failed\n");
        return 1;
    }

    // JWT claim 설정 
    jwt_add_grant(jwt, "iss", "admin_chan"); 
    jwt_add_grant(jwt, "sub", id);
    jwt_add_grant_int(jwt, "exp", time(NULL) + expired_time);


    // HS256 암호화 알고리즘 적용하여 인코딩 
    jwt_set_alg(jwt, JWT_ALG_HS256, (const unsigned char*)JWT_SECRET_KEY, strlen(JWT_SECRET_KEY));
    encoded = jwt_encode_str(jwt);

    if (!encoded) {
        fprintf(stderr, "jwt_encode_str() failed\n");
        jwt_free(jwt);
        return FAIL;
    }
    
    printf("Generated JWT: %s\n", encoded);
    *out = encoded;

    long exp        = jwt_get_grant_int(jwt, "exp");

    jwt_free(jwt);
    return exp;
}

int validate_jwt(char* token){
    jwt_t *jwt = NULL;

    // 토큰을 검사하여 유효한지 판별
    if (jwt_decode(&jwt, token,
                   (const unsigned char*)JWT_SECRET_KEY, strlen(JWT_SECRET_KEY)) != 0) {
        fprintf(stderr, "Invalid or tampered JWT\n");
        return FAIL;
    }

    const char *iss = jwt_get_grant(jwt, "iss");
    const char *sub = jwt_get_grant(jwt, "sub");
    long exp        = jwt_get_grant_int(jwt, "exp");

    printf("iss: %s\n", iss);
    printf("sub: %s\n", sub);
    printf("exp: %ld (now: %ld)\n", exp, time(NULL));

    jwt_free(jwt);
    return SUCCESS;
}