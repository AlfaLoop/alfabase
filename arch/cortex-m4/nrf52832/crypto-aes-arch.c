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
#include "crypto/aes.h"
#include "errno.h"
#include "nrf_soc.h"
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

// hardware accelation
static nrf_ecb_hal_data_t   m_ecb_data;
/*---------------------------------------------------------------------------*/
int
aes_ecb_encrypt_arch(uint8_t *key, uint8_t *input, uint8_t *output)
{
  memset(m_ecb_data.ciphertext, 0, SOC_ECB_KEY_LENGTH);
  memcpy(m_ecb_data.key, key, SOC_ECB_KEY_LENGTH);
  memcpy(m_ecb_data.cleartext, input, SOC_ECB_KEY_LENGTH);
  sd_ecb_block_encrypt(&m_ecb_data);
  memcpy(output, m_ecb_data.ciphertext, SOC_ECB_KEY_LENGTH);
  return ENONE;
}
/*---------------------------------------------------------------------------*/
