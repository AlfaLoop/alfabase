/**
 * © Copyright AlfaLoop Technology Co., Ltd. 2018
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/
#ifndef _HMAC_SHA256_H_
#define _HMAC_SHA256_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include "bsp_init.h"

#define CRYPTO_SHA256_BLOCK_SIZE (64)
#define CRYPTO_SHA256_DIGEST_SIZE (32)
#define CRYPTO_SHA256_STATE_BLOCKS (CRYPTO_SHA256_DIGEST_SIZE / 4 )

typedef struct {
  uint8_t  leftover[CRYPTO_SHA256_BLOCK_SIZE];
  uint32_t leftover_offset;
  uint64_t bits_hashed;
  uint32_t iv[CRYPTO_SHA256_STATE_BLOCKS];
} sha256_t;

int sha256_init(sha256_t *sha);

int sha256_update(sha256_t *sha,
                   const uint8_t *data,
                   uint32_t datalen);

int sha256_final(sha256_t *sha, uint8_t *digest);

typedef struct {
    uint8_t	  key[2*CRYPTO_SHA256_BLOCK_SIZE];
    sha256_t	sha;
} hmac_sha256_t;

int hmac_sha256_init(hmac_sha256_t *hmac, const uint8_t *key,
                                              uint32_t datalen);

int hmac_sha256_update(hmac_sha256_t *hmac, const void *data,
                                                  uint32_t datalen);

int hmac_sha256_final(hmac_sha256_t *hmac, uint8_t *data);


#ifdef __cplusplus
}
#endif
#endif /* _HMAC_SHA256_H_ */
