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
#include "oscrypto_api.h"
#include "frameworks/app_lifecycle.h"
#include "frameworks/app_eventpool.h"
#include "loader/symtab.h"
#include "crypto/hmac-sha256.h"
#include "crypto/aes.h"
#include "errno.h"
/*---------------------------------------------------------------------------*/
#if defined(DEBUG_ENABLE)
#define DEBUG_MODULE 0
#if DEBUG_MODULE
#include "dev/syslog.h"
#define PRINTF(...) syslog(__VA_ARGS__)
#else
#define PRINTF(...)
#endif  /* DEBUG_MODULE */
#else
#define PRINTF(...)
#endif  /* DEBUG_ENABLE */
/*---------------------------------------------------------------------------*/
static int
aes_ecb_encrypt_api(uint8_t *key, uint8_t *input, uint8_t *output)
{
#if CRYPTO_AES_ECB_CONF == 1
  return aes_ecb_encrypt(key, input, output);
#else
  return ENOSUPPORT;
#endif
}
/*---------------------------------------------------------------------------*/
static int
aes_ecb_decrypt_api(uint8_t *key, uint8_t *input, uint8_t *output)
{
#if CRYPTO_AES_ECB_CONF == 1
  return aes_ecb_decrypt(key, input, output);
#else
  return ENOSUPPORT;
#endif
}
/*---------------------------------------------------------------------------*/
static int
sha256_init_api(Sha256 *sha)
{
#if CRYPTO_SHA256_CONF == 1
  return sha256_init(sha);
#else
  return ENOSUPPORT;
#endif
}
/*---------------------------------------------------------------------------*/
static int
sha256_update_api(Sha256 *sha, const uint8_t *data, uint32_t datalen)
{
#if CRYPTO_SHA256_CONF == 1
  sha256_update(sha, data, datalen);
#else
  return ENOSUPPORT;
#endif
}
/*---------------------------------------------------------------------------*/
static int
sha256_final_api(Sha256 *sha, uint8_t *digest)
{
#if CRYPTO_SHA256_CONF == 1
  sha256_final(sha, digest);
#else
  return ENOSUPPORT;
#endif
}
/*---------------------------------------------------------------------------*/
static int
hmac_sha256_init_api(HmacSHA256 *hmac, const uint8_t *key, uint32_t datalen)
{
#if CRYPTO_HMAC_SHA256_CONF == 1 && CRYPTO_SHA256_CONF == 1
  return hmac_sha256_init(hmac, key, datalen);
#else
  return ENOSUPPORT;
#endif
}
/*---------------------------------------------------------------------------*/
static int
hmac_sha256_update_api(HmacSHA256 *hmac, const void *data, uint32_t datalen)
{
#if CRYPTO_HMAC_SHA256_CONF == 1 && CRYPTO_SHA256_CONF == 1
  return hmac_sha256_update(hmac, data, datalen);
#else
  return ENOSUPPORT;
#endif
}
/*---------------------------------------------------------------------------*/
static int
hmac_sha256_final_api(HmacSHA256 *hmac, uint8_t *digest)
{
#if CRYPTO_HMAC_SHA256_CONF == 1 && CRYPTO_SHA256_CONF == 1
  return hmac_sha256_final(hmac, digest);
#else
  return ENOSUPPORT;
#endif
}
/*---------------------------------------------------------------------------*/
Crypto*
OSCrypto(void)
{
	static Crypto instance;
  instance.aesECBEncrypt = aes_ecb_encrypt_api;
  instance.aesECBDecrypt = aes_ecb_decrypt_api;
  instance.sha256Init = sha256_init_api;
  instance.sha256Update = sha256_update_api;
  instance.sha256Final = sha256_final_api;
  instance.hmacSha256Init = hmac_sha256_init_api;
  instance.hmacSha256Update = hmac_sha256_update_api;
  instance.hmacSha256Final = hmac_sha256_final_api;
	return &instance;
}
static struct symbols symbolOSCrypto = {
	.name = "OSCrypto",
	.value = (void *)&OSCrypto
};
/*---------------------------------------------------------------------------*/
static void
app_terminating(void)
{
}
/*---------------------------------------------------------------------------*/
static struct app_lifecycle_event lifecycle_event = {
	.name = "oscrypto_api",
	.terminating = app_terminating
};
/*---------------------------------------------------------------------------*/
void
oscrypto_api_init(void)
{
  app_lifecycle_register(&lifecycle_event);
	symtab_add(&symbolOSCrypto);
}
/*---------------------------------------------------------------------------*/
