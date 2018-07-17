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
#include "contiki.h"
#include "nest/nest.h"
#include "nest-driver.h"
#include "nrf.h"
#include "nrf_soc.h"
#include "nrf_sdm.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_types.h"
#include "nordic_common.h"
#include "softdevice_handler.h"
#include "spiffs-arch.h"
/*---------------------------------------------------------------------------*/
#if defined(DEBUG_ENABLE)
#define DEBUG_MODULE 1
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
/**@brief Set the radio's transmit power.
 *
 * @param[in] tx_power Radio transmit power in dBm (accepted values are -40, -30, -20, -16, -12, -8, -4, 0, 3, and 4 dBm).
 *
 * @note The +3dBm setting is only available on nRF52 series ICs.
 * @note The -30dBm setting is only available on nRF51 series ICs.
 * @note The -40dBm setting is only available on nRF52 series ICs.
 *
 * @retval ::NRF_SUCCESS Successfully changed the transmit power.
 * @retval ::NRF_ERROR_INVALID_PARAM Invalid parameter(s) supplied.
 */
// SVCALL(SD_BLE_GAP_TX_POWER_SET, uint32_t, sd_ble_gap_tx_power_set(int8_t tx_power));
int
nrf_gap_set_txpower(int tx_power)
{
  uint32_t err_code = NRF_SUCCESS;

  if (tx_power <= -30) {
    tx_power = -30;
  } else if (tx_power <= -20) {
    tx_power = -20;
  } else if (tx_power <= -16) {
    tx_power = -16;
  } else if (tx_power <= -12) {
    tx_power = -12;
  } else if (tx_power <= -8) {
    tx_power = -8;
  } else if (tx_power <= -4) {
    tx_power = -4;
  } else if (tx_power <= 0) {
    tx_power = 0;
  } else if (tx_power <= 4) {
    tx_power = 4;
  } else {
    tx_power = 0;
  }

  err_code = sd_ble_gap_tx_power_set(tx_power);
  if (err_code == NRF_SUCCESS) {
    err_code = ENONE;
  } else if (err_code == NRF_ERROR_INVALID_PARAM) {
    err_code = EINVAL;
  }
  return err_code;
}
/*---------------------------------------------------------------------------*/
