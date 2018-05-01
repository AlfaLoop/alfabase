/**
 *  Copyright (c) 2016 AlfaLoop Technology Co., Ltd. All Rights Reserved.
 *
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential.
 *
 *  Attribution - You must give appropriate credit, provide a link to the license, and
 *  indicate if changes were made. You may do so in any reasonable manner, but not in any
 *  way that suggests the licensor endorses you or your use.
 *
 *  NonCommercial - You may not use the material for commercial purposes under unauthorized.
 *
 *  NoDerivatives - If you remix, transform, or build upon the material, you may not
 *  distribute the modified material.
 */
#ifndef _HMAC_SHA256_H_
#define _HMAC_SHA256_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#define CRYPTO_SHA256_BLOCK_SIZE (64)
#define CRYPTO_SHA256_DIGEST_SIZE (32)
#define CRYPTO_SHA256_STATE_BLOCKS (CRYPTO_SHA256_DIGEST_SIZE / 4 )

typedef struct {
  uint8_t  leftover[CRYPTO_SHA256_BLOCK_SIZE];
  uint32_t leftover_offset;
  uint64_t bits_hashed;
  uint32_t iv[CRYPTO_SHA256_STATE_BLOCKS];
} sha256_t;

uint32_t sha256_init(sha256_t *sha);

uint32_t sha256_update(sha256_t *sha,
                   const uint8_t *data,
                   uint32_t datalen);

uint32_t sha256_final(sha256_t *sha, uint8_t *digest);

typedef struct {
    uint8_t	  key[2*CRYPTO_SHA256_BLOCK_SIZE];
    sha256_t	sha;
} hmac_sha256_t;

uint32_t hmac_sha256_setkey(hmac_sha256_t *hmac, const uint8_t *key,
                                              uint32_t datalen);

uint32_t hmac_sha256_init(hmac_sha256_t *hmac);

uint32_t hmac_sha256_update(hmac_sha256_t *hmac, const void *data,
                                                  uint32_t datalen);

uint32_t hmac_sha256_final(hmac_sha256_t *hmac, uint8_t *data);


#ifdef __cplusplus
}
#endif
#endif /* _HMAC_SHA256_H_ */
