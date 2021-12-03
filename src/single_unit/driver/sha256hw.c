#include "sha256hw.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

static volatile unsigned char *sha256_hw_mem = NULL;   // mmio via /dev/mem

#define hw_read(offset) (*(volatile uint32_t *)((sha256_hw_mem) + 4*(offset)))
#define hw_write(offset, data) (*(volatile uint32_t *)((sha256_hw_mem) + 4*(offset)) = (data))


// #define hw_read(offset)           (((uint32_t*)sha256_hw_mem)[4*(offset)])
// #define hw_write(offset, data)    (((uint32_t*)sha256_hw_mem)[4*(offset)] = (data))

void sha256_hw_mmap()
{
    printf("%s", "mmmap called\n");
    off_t offset = SHA256_HW_BASE_ADDR;
    size_t len = 11*4;

    // Truncate offset to a multiple of the page size, or mmap will fail.
    size_t pagesize = sysconf(_SC_PAGE_SIZE);
    off_t page_base = (offset / pagesize) * pagesize;
    off_t page_offset = offset - page_base;

    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    sha256_hw_mem = mmap(NULL, page_offset + len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, page_base);
    sha256_hw_mem += page_offset;
}

int sha256_hw_blkready() 
{
    printf("%s", "blkready called\n");
    return 0x00000001 & hw_read(SHA256_HW_BRDY_OFFSET);
}

void sha256_hw_wait() 
{
    printf("%s", "wait called\n");
    while(!sha256_hw_blkready());
}
void sha256_hw_data_wr(uint32_t data) 
{
    printf("%s", "data_wr called ");
    hw_write(SHA256_HW_DATA_OFFSET,data);
    printf("%x\n", hw_read(SHA256_HW_DATA_OFFSET));

}

void sha256_hw_reset()
{
    printf("%s", "reset called\n");
    hw_write(SHA256_HW_RSTN_OFFSET, 0x00000000);
    hw_write(SHA256_HW_RSTN_OFFSET, 0x00000001);
}

void sha256_hw_get_hash(SHA256_CTX *ctx)
{
    printf("%s", "get_hash called\n");
    sha256_hw_wait();   // wait until hash is ready
    for (int i = 0; i < 8; i++)
        ctx->h[i] = hw_read(SHA256_HW_HASH_OFFSET+i);
}

// modified openssl sha256
// sha.h https://github.com/openssl/openssl/blob/807bb4255473986439c00a2fe3c798e7120709ba/include/openssl/sha.h
// sha.c https://github.com/openssl/openssl/blob/657d1927c68bdc3fb0250d16df2a8439e8e043f1/crypto/sha/sha256.c
// md32_common.h https://github.com/openssl/openssl/blob/1c0eede9827b0962f1d752fa4ab5d436fa039da4/include/crypto/md32_common.h
// mem_clr.c https://github.com/openssl/openssl/blob/1c0eede9827b0962f1d752fa4ab5d436fa039da4/crypto/mem_clr.c

// below are from mem_clr.c

typedef void *(*memset_t)(void *, int, size_t);

static volatile memset_t memset_func = memset;

void OPENSSL_cleanse(void *ptr, size_t len)
{
    memset_func(ptr, 0, len);
}

// below are modified from sha.c

int SHA256_Init(SHA256_CTX *c)
{
    if (sha256_hw_mem == NULL) sha256_hw_mmap();
    memset(c, 0, sizeof(*c));
    // c->h[0] = 0x6a09e667UL;
    // c->h[1] = 0xbb67ae85UL;
    // c->h[2] = 0x3c6ef372UL;
    // c->h[3] = 0xa54ff53aUL;
    // c->h[4] = 0x510e527fUL;
    // c->h[5] = 0x9b05688cUL;
    // c->h[6] = 0x1f83d9abUL;
    // c->h[7] = 0x5be0cd19UL;
    sha256_hw_reset();
    c->md_len = SHA256_DIGEST_LENGTH;
    return 1;
}

void sha256_block_data_order(SHA256_CTX *ctx, const void *in, size_t num)
{
    if (sha256_hw_mem == NULL) sha256_hw_mmap();
    // unsigned MD32_REG_T a, b, c, d, e, f, g, h, s0, s1, T1, T2;
    // SHA_LONG X[16], l;
    SHA_LONG l;
    int i;
    const unsigned char *data = in;

    while (num--) {

        // a = ctx->h[0];
        // b = ctx->h[1];
        // c = ctx->h[2];
        // d = ctx->h[3];
        // e = ctx->h[4];
        // f = ctx->h[5];
        // g = ctx->h[6];
        // h = ctx->h[7];

        sha256_hw_wait();   // may not be needed because of fifo but still
        for (i = 0; i < 16; i++) {
            (void)HOST_c2l(data, l);    // this macro increments data pointer
            sha256_hw_data_wr(l);
            // T1 = X[i] = l;
            // T1 += h + Sigma1(e) + Ch(e, f, g) + K256[i];
            // T2 = Sigma0(a) + Maj(a, b, c);
            // h = g;
            // g = f;
            // f = e;
            // e = d + T1;
            // d = c;
            // c = b;
            // b = a;
            // a = T1 + T2;
        }

        // for (; i < 64; i++) {
        //     s0 = X[(i + 1) & 0x0f];
        //     s0 = sigma0(s0);
        //     s1 = X[(i + 14) & 0x0f];
        //     s1 = sigma1(s1);

        //     T1 = X[i & 0xf] += s0 + s1 + X[(i + 9) & 0xf];
        //     T1 += h + Sigma1(e) + Ch(e, f, g) + K256[i];
        //     T2 = Sigma0(a) + Maj(a, b, c);
        //     h = g;
        //     g = f;
        //     f = e;
        //     e = d + T1;
        //     d = c;
        //     c = b;
        //     b = a;
        //     a = T1 + T2;
        // }

        // ctx->h[0] += a;
        // ctx->h[1] += b;
        // ctx->h[2] += c;
        // ctx->h[3] += d;
        // ctx->h[4] += e;
        // ctx->h[5] += f;
        // ctx->h[6] += g;
        // ctx->h[7] += h;

    }
    sha256_hw_get_hash(ctx);
}

// below are from md32_common.h

int HASH_UPDATE(HASH_CTX *c, const void *data_, size_t len)
{
    const unsigned char *data = data_;
    unsigned char *p;
    HASH_LONG l;
    size_t n;

    if (len == 0)
        return 1;

    l = (c->Nl + (((HASH_LONG) len) << 3)) & 0xffffffffUL;
    if (l < c->Nl)              /* overflow */
        c->Nh++;
    c->Nh += (HASH_LONG) (len >> 29); /* might cause compiler warning on
                                       * 16-bit */
    c->Nl = l;

    n = c->num;
    if (n != 0) {
        p = (unsigned char *)c->data;

        if (len >= HASH_CBLOCK || len + n >= HASH_CBLOCK) {
            memcpy(p + n, data, HASH_CBLOCK - n);
            HASH_BLOCK_DATA_ORDER(c, p, 1);
            n = HASH_CBLOCK - n;
            data += n;
            len -= n;
            c->num = 0;
            /*
             * We use memset rather than OPENSSL_cleanse() here deliberately.
             * Using OPENSSL_cleanse() here could be a performance issue. It
             * will get properly cleansed on finalisation so this isn't a
             * security problem.
             */
            memset(p, 0, HASH_CBLOCK); /* keep it zeroed */
        } else {
            memcpy(p + n, data, len);
            c->num += (unsigned int)len;
            return 1;
        }
    }

    n = len / HASH_CBLOCK;
    if (n > 0) {
        HASH_BLOCK_DATA_ORDER(c, data, n);
        n *= HASH_CBLOCK;
        data += n;
        len -= n;
    }

    if (len != 0) {
        p = (unsigned char *)c->data;
        c->num = (unsigned int)len;
        memcpy(p, data, len);
    }
    return 1;
}

void HASH_TRANSFORM(HASH_CTX *c, const unsigned char *data)
{
    HASH_BLOCK_DATA_ORDER(c, data, 1);
}

int HASH_FINAL(unsigned char *md, HASH_CTX *c)
{
    unsigned char *p = (unsigned char *)c->data;
    size_t n = c->num;

    p[n] = 0x80;                /* there is always room for one */
    n++;

    if (n > (HASH_CBLOCK - 8)) {
        memset(p + n, 0, HASH_CBLOCK - n);
        n = 0;
        HASH_BLOCK_DATA_ORDER(c, p, 1);
    }
    memset(p + n, 0, HASH_CBLOCK - 8 - n);

    p += HASH_CBLOCK - 8;
#if   defined(DATA_ORDER_IS_BIG_ENDIAN)
    (void)HOST_l2c(c->Nh, p);
    (void)HOST_l2c(c->Nl, p);
#elif defined(DATA_ORDER_IS_LITTLE_ENDIAN)
    (void)HOST_l2c(c->Nl, p);
    (void)HOST_l2c(c->Nh, p);
#endif
    p -= HASH_CBLOCK;
    HASH_BLOCK_DATA_ORDER(c, p, 1);
    c->num = 0;
    OPENSSL_cleanse(p, HASH_CBLOCK);

#ifndef HASH_MAKE_STRING
# error "HASH_MAKE_STRING must be defined!"
#else
    HASH_MAKE_STRING(c, md);
#endif

    return 1;
}