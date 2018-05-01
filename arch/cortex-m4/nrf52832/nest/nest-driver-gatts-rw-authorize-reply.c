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
int
nrf_gatts_rw_authorize_reply(uint16_t conn_id, uint16_t handle, uint16_t offset, uint16_t length, uint8_t const *p_value)
{
  uint32_t err_code;
  uint8_t  txbuf[BLE_NEST_FIX_DATA_LEN];

  memcpy(txbuf, p_value, BLE_NEST_FIX_DATA_LEN);

  const ble_gattc_write_params_t write_params = {
    .write_op = BLE_GATT_OP_WRITE_CMD,
    .flags    = BLE_GATT_EXEC_WRITE_FLAG_PREPARED_WRITE,
    .handle   = handle,
    .offset   = 0,
    .len      = length,
    .p_value  = txbuf
  };

  err_code = sd_ble_gatts_rw_authorize_reply(conn_id, &auth_reply);
  if (err_code != NRF_SUCCESS) {
    PRINTF("nrf gattc write error %d \n", err_code);
    if (err_code == BLE_ERROR_INVALID_CONN_HANDLE) {
      return EINVAL;
    } else if (err_code == NRF_ERROR_INVALID_STATE) {
      return EINVALSTATE;
    } else if (err_code == NRF_ERROR_INVALID_ADDR) {
      return EFAULT;
    } else if (err_code == NRF_ERROR_INVALID_PARAM) {
      return EINVAL;
    } else if (err_code == NRF_ERROR_DATA_SIZE) {
      return EINVAL;
    } else if (err_code == NRF_ERROR_BUSY) {
      return EBUSY;
    } else if (err_code == BLE_ERROR_NO_TX_PACKETS) {
      return ELOAR;
    }
  }
  return ENONE;
}
/*---------------------------------------------------------------------------*/
