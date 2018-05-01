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
nrf_gatts_handle_value(uint16_t conn_id, uint8_t type,
                       uint16_t handle, uint8_t *data, uint16_t length)
{
  uint32_t err_code;
  uint8_t txbuf[BLE_NEST_FIX_DATA_LEN];
  ble_gatts_hvx_params_t hvx_params;

  if (data == NULL)
    return ENULLP;

  memcpy(txbuf, data, BLE_NEST_FIX_DATA_LEN);

  memset(&hvx_params, 0, sizeof(hvx_params));
  hvx_params.handle = handle;
  hvx_params.p_data = txbuf;
  hvx_params.p_len  = &length;
  if (type == NEST_BLE_HVX_TYPE_NOTIFICATION)
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
  else if (type == NEST_BLE_HVX_TYPE_INDICATION)
    hvx_params.type   = BLE_GATT_HVX_INDICATION;

  err_code = sd_ble_gatts_hvx(conn_id, &hvx_params);
  if (err_code != NRF_SUCCESS) {
    PRINTF("nest channel output error %d \n", err_code);
    if (err_code == BLE_ERROR_INVALID_CONN_HANDLE) {
      return EINVAL;
    } else if (err_code == NRF_ERROR_INVALID_STATE) {
      return EINVALSTATE;
    } else if (err_code == NRF_ERROR_INVALID_ADDR) {
      return EFAULT;
    } else if (err_code == NRF_ERROR_INVALID_PARAM) {
      return EINVAL;
    } else if (err_code == BLE_ERROR_INVALID_ATTR_HANDLE) {
      return EINVAL;
    } else if (err_code == BLE_ERROR_GATTS_INVALID_ATTR_TYPE) {
      return EINVAL;
    } else if (err_code == NRF_ERROR_NOT_FOUND) {
      return ENOFOUND;
    } else if (err_code == NRF_ERROR_FORBIDDEN) {
      return EPERM;
    } else if (err_code == NRF_ERROR_DATA_SIZE) {
      return EINVAL;
    } else if (err_code == NRF_ERROR_BUSY) {
      return EBUSY;
    } else if (err_code == BLE_ERROR_GATTS_SYS_ATTR_MISSING) {
      return EINTERNAL;
    } else if (err_code == BLE_ERROR_NO_TX_PACKETS) {
      return ELOAR;
    }
  }
  return ENONE;
}
/*---------------------------------------------------------------------------*/
