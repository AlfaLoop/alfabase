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

/**@brief Set GAP device name.
 *
 * @param[in] p_write_perm Write permissions for the Device Name characteristic, see @ref ble_gap_conn_sec_mode_t.
 * @param[in] p_dev_name Pointer to a UTF-8 encoded, <b>non NULL-terminated</b> string.
 * @param[in] len Length of the UTF-8, <b>non NULL-terminated</b> string pointed to by p_dev_name in octets (must be smaller or equal than @ref BLE_GAP_DEVNAME_MAX_LEN).
 *
 * @note If the device name is located in application flash memory (see @ref ble_gap_device_name_t), it cannot be changed. Then @ref NRF_ERROR_FORBIDDEN will be returned.
 *
 * @retval ::NRF_SUCCESS GAP device name and permissions set successfully.
 * @retval ::NRF_ERROR_INVALID_ADDR Invalid pointer supplied.
 * @retval ::NRF_ERROR_INVALID_PARAM Invalid parameter(s) supplied.
 * @retval ::NRF_ERROR_DATA_SIZE Invalid data size(s) supplied.
 * @retval ::NRF_ERROR_FORBIDDEN Device name is not writable.
 */
// SVCALL(SD_BLE_GAP_DEVICE_NAME_SET, uint32_t, sd_ble_gap_device_name_set(ble_gap_conn_sec_mode_t const *p_write_perm, uint8_t const *p_dev_name, uint16_t len));
/*---------------------------------------------------------------------------*/
int
nrf_gap_set_device_name(const char *name, int length)
{
  uint32_t err_code = NRF_SUCCESS;

  ble_gap_conn_sec_mode_t sec_mode;
  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
  err_code = sd_ble_gap_device_name_set(&sec_mode,
                                        (const uint8_t *)name,
                                        length);
  if (err_code == NRF_ERROR_INVALID_ADDR) {
    err_code = EFAULT;
  } else if (err_code == NRF_ERROR_INVALID_PARAM) {
    err_code = EINVAL;
  } else if (err_code == NRF_ERROR_DATA_SIZE) {
    err_code = EOVERFLOW;
  } else if (err_code == NRF_ERROR_FORBIDDEN) {
    err_code = EPERM;
  }
  return err_code;
}
/*---------------------------------------------------------------------------*/
