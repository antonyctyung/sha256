#include <stdio.h>
#include "sha256hw.h"

int main()
{
    unsigned char md[SHA256_DIGEST_LENGTH];
    SHA256_CTX ctx;
    size_t message_len = 11;
    unsigned char message[] = "hello world";
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, message, message_len);
    SHA256_Final(md, &ctx);
    printf("%s",md);
}