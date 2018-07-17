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
static ble_gap_addr_t initiate_address;
static void (* gap_connect_timeout_callback)(void) = NULL;
/*---------------------------------------------------------------------------*/
int
nrf_gap_connect(nest_device_t *device,  void (* timeout_callback)(void) )
{
  uint32_t    err_code;
  ble_gap_scan_params_t scan_params;
  ble_gap_conn_params_t connection_param;

  if (device == NULL) {
    return ENULLP;
  }

  scan_params.active   = 1;
#if NRF_SD_BLE_API_VERSION == 3
  scan_params.use_whitelist = 0;
  scan_params.adv_dir_report = 0;
#elif NRF_SD_BLE_API_VERSION == 2
  scan_params.selective   = 0;
  scan_params.p_whitelist = NULL;
#endif

  // Source: https://devzone.nordicsemi.com/f/nordic-q-a/10386/sd_ble_gap_connect-timeout-in-s120-central
  // non-zero timeout value in the scan parameters provided as a parameter for sd_ble_gap_connect.
  // This parameter represents the timeout in seconds.
  // Once you have set the timeout parameter, you will either receive an BLE_GAP_EVT_CONNECTED event, or BLE_GAP_EVT_TIMEOUT event with the source set to 'connection'.
  scan_params.interval = NEST_SCAN_INTERVAL;
  scan_params.window   = NEST_SCAN_WINDOW;
  scan_params.timeout  = NEST_SCAN_TIMEOUT;

  connection_param.min_conn_interval = (uint16_t) MSEC_TO_UNITS(NEST_MIN_CONNECTION_INTERVAL, UNIT_1_25_MS);
  connection_param.max_conn_interval = (uint16_t) MSEC_TO_UNITS(NEST_MAX_CONNECTION_INTERVAL, UNIT_1_25_MS);
  connection_param.slave_latency = 0;
  connection_param.conn_sup_timeout = (uint16_t)MSEC_TO_UNITS(NEST_SUPERVISION_TIMEOUT, UNIT_10_MS);

  initiate_address.addr_type = device->type;
  memcpy(initiate_address.addr, device->address, 6);
#if NRF_SD_BLE_API_VERSION == 3
  initiate_address.addr_id_peer = 0;
#endif

  gap_connect_timeout_callback = timeout_callback;
  err_code = sd_ble_gap_connect(&initiate_address,
                                &scan_params,
                                &connection_param);
  if (err_code != NRF_SUCCESS) {
    // PRINTF("[nest driver gap connect] conn failed, reason %d\n", err_code);
    if (err_code == NRF_ERROR_INVALID_ADDR) {
      err_code = EFAULT;
    } else if (err_code == NRF_ERROR_INVALID_PARAM) {
      err_code = EINVAL;
    } else if (err_code == NRF_ERROR_INVALID_STATE) {
      err_code = EINVALSTATE;
    } else if (err_code == BLE_ERROR_GAP_INVALID_BLE_ADDR) {
      err_code = EINVAL;
    } else if (err_code == NRF_ERROR_CONN_COUNT) {
      err_code = EBUSY;
    } else if (err_code == NRF_ERROR_BUSY) {
      err_code = EBUSY;
    } else if (err_code == NRF_ERROR_NO_MEM) {
      err_code = ENOMEM;
    } else if (err_code == NRF_ERROR_RESOURCES) {
      err_code = ELOAR;
    }
  } else {
    err_code = ENONE;
  }
  return err_code;
}
/*---------------------------------------------------------------------------*/
void
nrf_gap_central_connect_timeout(void)
{
  if (gap_connect_timeout_callback != NULL) {
    gap_connect_timeout_callback();
  }
}
/*---------------------------------------------------------------------------*/
