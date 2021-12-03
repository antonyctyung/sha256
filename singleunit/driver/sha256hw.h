#ifndef _SHA256HW_H
#define _SHA256HW_H

#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

// modified openssl sha256
// sha.h https://github.com/openssl/openssl/blob/807bb4255473986439c00a2fe3c798e7120709ba/include/openssl/sha.h
// sha.c https://github.com/openssl/openssl/blob/657d1927c68bdc3fb0250d16df2a8439e8e043f1/crypto/sha/sha256.c
// md32_common.h https://github.com/openssl/openssl/blob/1c0eede9827b0962f1d752fa4ab5d436fa039da4/include/crypto/md32_common.h
// mem_clr.c https://github.com/openssl/openssl/blob/1c0eede9827b0962f1d752fa4ab5d436fa039da4/crypto/mem_clr.c

# define SHA224_DIGEST_LENGTH    28
# define SHA256_DIGEST_LENGTH    32

#define DATA_ORDER_IS_BIG_ENDIAN

#define SHA_LONG        unsigned long
#define SHA_LBLOCK      16

/* SHA treats input data as a
 * contiguous array of 32 bit wide
 * big-endian values. */
#define SHA_CBLOCK      (SHA_LBLOCK*4)

/* SHA-256 treats input data as a
 * contiguous array of 32 bit wide
 * big-endian values. */
#define SHA256_CBLOCK   (SHA_LBLOCK*4)

#define HASH_LONG               SHA_LONG
#define HASH_CTX                SHA256_CTX
#define HASH_CBLOCK             SHA_CBLOCK

#define HASH_MAKE_STRING(c,s)   do {    \
        unsigned long ll;               \
        unsigned int  nn;               \
        switch ((c)->md_len)            \
        {   case SHA224_DIGEST_LENGTH:  \
                for (nn=0;nn<SHA224_DIGEST_LENGTH/4;nn++)       \
                {   ll=(c)->h[nn]; (void)HOST_l2c(ll,(s));   }  \
                break;                  \
            case SHA256_DIGEST_LENGTH:  \
                for (nn=0;nn<SHA256_DIGEST_LENGTH/4;nn++)       \
                {   ll=(c)->h[nn]; (void)HOST_l2c(ll,(s));   }  \
                break;                  \
            default:                    \
                if ((c)->md_len > SHA256_DIGEST_LENGTH) \
                    return 0;                           \
                for (nn=0;nn<(c)->md_len/4;nn++)                \
                {   ll=(c)->h[nn]; (void)HOST_l2c(ll,(s));   }  \
                break;                  \
        }                               \
        } while (0)

#define HASH_UPDATE             SHA256_Update
#define HASH_TRANSFORM          SHA256_Transform
#define HASH_FINAL              SHA256_Final
#define HASH_BLOCK_DATA_ORDER   sha256_block_data_order

#if defined(DATA_ORDER_IS_BIG_ENDIAN)

# define HOST_c2l(c,l)  (l =(((unsigned long)(*((c)++)))<<24),          \
                         l|=(((unsigned long)(*((c)++)))<<16),          \
                         l|=(((unsigned long)(*((c)++)))<< 8),          \
                         l|=(((unsigned long)(*((c)++)))    )           )
# define HOST_l2c(l,c)  (*((c)++)=(unsigned char)(((l)>>24)&0xff),      \
                         *((c)++)=(unsigned char)(((l)>>16)&0xff),      \
                         *((c)++)=(unsigned char)(((l)>> 8)&0xff),      \
                         *((c)++)=(unsigned char)(((l)    )&0xff),      \
                         l)

#elif defined(DATA_ORDER_IS_LITTLE_ENDIAN)

# define HOST_c2l(c,l)  (l =(((unsigned long)(*((c)++)))    ),          \
                         l|=(((unsigned long)(*((c)++)))<< 8),          \
                         l|=(((unsigned long)(*((c)++)))<<16),          \
                         l|=(((unsigned long)(*((c)++)))<<24)           )
# define HOST_l2c(l,c)  (*((c)++)=(unsigned char)(((l)    )&0xff),      \
                         *((c)++)=(unsigned char)(((l)>> 8)&0xff),      \
                         *((c)++)=(unsigned char)(((l)>>16)&0xff),      \
                         *((c)++)=(unsigned char)(((l)>>24)&0xff),      \
                         l)

#endif

#define SHA256_HW_BASE_ADDR     0x43C00000

// WORD offset i.e. 4 bytes
#define SHA256_HW_HASH_OFFSET   0
#define SHA256_HW_DATA_OFFSET   8
#define SHA256_HW_BRDY_OFFSET   9
#define SHA256_HW_RSTN_OFFSET   10

typedef struct SHA256state_st {
    SHA_LONG h[8];
    SHA_LONG Nl, Nh;
    SHA_LONG data[SHA_LBLOCK];
    unsigned int num, md_len;
} SHA256_CTX;

void sha256_hw_mmap();
int sha256_hw_blkready();
void sha256_hw_wait();
void sha256_hw_data_wr(uint32_t data);
void sha256_hw_reset();
void sha256_hw_get_hash(SHA256_CTX *ctx);

int SHA256_Init(SHA256_CTX *c);
void sha256_block_data_order(SHA256_CTX *ctx, const void *in, size_t num);
int SHA256_Update(SHA256_CTX *c, const void *data, size_t len);
void SHA256_Transform(SHA256_CTX *c, const unsigned char *data);
int SHA256_Final(unsigned char *md, SHA256_CTX *c);

#ifdef __cplusplus
}
#endif

#endif