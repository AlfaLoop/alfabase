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
#ifndef _AES_H_
#define _AES_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include "bsp_init.h"

#if CRYPTO_AES_ECB_CONF == 1

int aes_ecb_encrypt(uint8_t *key, uint8_t *input, uint8_t *output);
int aes_ecb_decrypt(uint8_t *key, uint8_t *input, uint8_t *output);

#if CRYPTO_AES_ECB_ENCRYPT_HW_CONF == 1
int aes_ecb_encrypt_arch(uint8_t *key, uint8_t *input, uint8_t *output);
#endif /* CRYPTO_AES_ECB_ENCRYPT_HW_CONF */
#if CRYPTO_AES_ECB_DECRYPT_HW_CONF == 1
int aes_ecb_decrypto_arch(uint8_t *key, uint8_t *input, uint8_t *output);
#endif /* CRYPTO_AES_ECB_DECRYPT_HW_CONF */

#endif /* CRYPTO_AES_ECB_CONF */

#ifdef __cplusplus
}
#endif
#endif /* _AES_H_ */
