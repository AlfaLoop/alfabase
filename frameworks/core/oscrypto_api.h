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
#ifndef __OSCRYPTO_API_H
#define __OSCRYPTO_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>


/* Framework API */
#define OSCRYPTO_SHA256_BLOCK_SIZE (64)
#define OSCRYPTO_SHA256_DIGEST_SIZE (32)
#define OSCRYPTO_SHA256_STATE_BLOCKS (OSCRYPTO_SHA256_DIGEST_SIZE / 4 )

typedef struct {
  uint8_t  leftover[OSCRYPTO_SHA256_BLOCK_SIZE];
  uint32_t leftover_offset;
  uint64_t bits_hashed;
  uint32_t iv[OSCRYPTO_SHA256_STATE_BLOCKS];
} Sha256;

typedef struct {
  uint8_t	 key[2*OSCRYPTO_SHA256_BLOCK_SIZE];
  Sha256	sha;
} HmacSHA256;

typedef struct{
  int (*aesECBEncrypt)(uint8_t *key, uint8_t *input, uint8_t *output);
  int (*aesECBDecrypt)(uint8_t *key, uint8_t *input, uint8_t *output);
  int (*sha256Init)(Sha256 *sha);
  int (*sha256Update)(Sha256 *sha, const uint8_t *data, uint32_t datalen);
  int (*sha256Final)(Sha256 *sha, uint8_t *digest);
  int (*hmacSha256Init)(HmacSHA256 *hmac, const uint8_t *key, uint32_t datalen);
  int (*hmacSha256Update)(HmacSHA256 *hmac, const void *data, uint32_t datalen);
  int (*hmacSha256Final)(HmacSHA256 *hmac, uint8_t *digest);
} Crypto;

Crypto* OSCrypto(void);
/* Framework API */


/* Back-end */
void oscrypto_api_init(void);
/* Back-end */


#ifdef __cplusplus
}
#endif
#endif /* __OSCRYPTO_API_H */
