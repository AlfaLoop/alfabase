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

/**@brief Set GAP Peripheral Preferred Connection Parameters.
 *
 * @param[in] p_conn_params Pointer to a @ref ble_gap_conn_params_t structure with the desired parameters.
 *
 * @retval ::NRF_SUCCESS Peripheral Preferred Connection Parameters set successfully.
 * @retval ::NRF_ERROR_INVALID_ADDR Invalid pointer supplied.
 * @retval ::NRF_ERROR_INVALID_PARAM Invalid parameter(s) supplied.
 */
// SVCALL(SD_BLE_GAP_PPCP_SET, uint32_t, sd_ble_gap_ppcp_set(ble_gap_conn_params_t const *p_conn_params));
int
nrf_gap_set_ppcp(const nest_connection_params_t *params)
{
  uint32_t    err_code = NRF_SUCCESS;

  static ble_gap_conn_params_t conn_params;
  conn_params.min_conn_interval = params->min_conn_interval;
  conn_params.max_conn_interval = params->max_conn_interval;
  conn_params.slave_latency = params->slave_latency;
  conn_params.conn_sup_timeout = params->conn_sup_timeout;

  err_code = sd_ble_gap_ppcp_set(&conn_params);
  if (err_code == NRF_ERROR_INVALID_ADDR) {
    PRINTF("[nest driver ppcp] set invalid addr\n");
    err_code = EFAULT;
  } else if (err_code == NRF_ERROR_INVALID_PARAM) {
    PRINTF("[nest driver ppcp] set invalid params\n");
    err_code = EINVAL;
  } else if (err_code == NRF_SUCCESS) {
    err_code = ENONE;
  }
  return err_code;
}
/*---------------------------------------------------------------------------*/
/**@brief Get GAP Peripheral Preferred Connection Parameters.
 *
 * @param[out] p_conn_params Pointer to a @ref ble_gap_conn_params_t structure where the parameters will be stored.
 *
 * @retval ::NRF_SUCCESS Peripheral Preferred Connection Parameters retrieved successfully.
 * @retval ::NRF_ERROR_INVALID_ADDR Invalid pointer supplied.
 */
// SVCALL(SD_BLE_GAP_PPCP_GET, uint32_t, sd_ble_gap_ppcp_get(ble_gap_conn_params_t *p_conn_params));
int
nrf_gap_get_ppcp(nest_connection_params_t *params)
{
  uint32_t err_code = NRF_SUCCESS;
  nest_connection_params_t conn_params;

  err_code = sd_ble_gap_ppcp_get(&conn_params);
  if (err_code == NRF_SUCCESS) {
    params->min_conn_interval = conn_params.min_conn_interval;
    params->max_conn_interval = conn_params.max_conn_interval;
    params->slave_latency = conn_params.slave_latency;
    params->conn_sup_timeout = conn_params.conn_sup_timeout;
    PRINTF("[nest driver ppcp] get min internval %d max internval %d latency %d timeout %d\n",
      params->min_conn_interval, params->max_conn_interval, params->slave_latency, params->conn_sup_timeout);
    err_code = ENONE;
  } else if (err_code == NRF_ERROR_INVALID_ADDR) {
    PRINTF("[nest driver ppcp] get invalid addr\n");
    err_code = EFAULT;
  }
  return err_code;
}
/*---------------------------------------------------------------------------*/
/**@brief Update connection parameters.
 *
 * @details In the central role this will initiate a Link Layer connection parameter update procedure,
 *          otherwise in the peripheral role, this will send the corresponding L2CAP request and wait for
 *          the central to perform the procedure. In both cases, and regardless of success or failure, the application
 *          will be informed of the result with a @ref BLE_GAP_EVT_CONN_PARAM_UPDATE event.
 *
 * @details This function can be used as a central both to reply to a @ref BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST or to start the procedure unrequested.
 *
 * @events
 * @event{@ref BLE_GAP_EVT_CONN_PARAM_UPDATE, Result of the connection parameter update procedure.}
 * @endevents
 *
 * @mscs
 * @mmsc{@ref BLE_GAP_CPU_MSC}
 * @mmsc{@ref BLE_GAP_CENTRAL_ENC_AUTH_MUTEX_MSC}
 * @mmsc{@ref BLE_GAP_MULTILINK_CPU_MSC}
 * @mmsc{@ref BLE_GAP_MULTILINK_CTRL_PROC_MSC}
 * @mmsc{@ref BLE_GAP_CENTRAL_CPU_MSC}
 * @endmscs
 *
 * @param[in] conn_handle Connection handle.
 * @param[in] p_conn_params  Pointer to desired connection parameters. If NULL is provided on a peripheral role,
 *                           the parameters in the PPCP characteristic of the GAP service will be used instead.
 *                           If NULL is provided on a central role and in response to a @ref BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST, the peripheral request will be rejected
 *
 * @retval ::NRF_SUCCESS The Connection Update procedure has been started successfully.
 * @retval ::NRF_ERROR_INVALID_ADDR Invalid pointer supplied.
 * @retval ::NRF_ERROR_INVALID_PARAM Invalid parameter(s) supplied, check parameter limits and constraints.
 * @retval ::NRF_ERROR_INVALID_STATE Invalid state to perform operation.
 * @retval ::NRF_ERROR_BUSY Procedure already in progress or not allowed at this time, process pending events and wait for pending procedures to complete and retry.
 * @retval ::BLE_ERROR_INVALID_CONN_HANDLE Invalid connection handle supplied.
 * @retval ::NRF_ERROR_NO_MEM Not enough memory to complete operation.
 */
// SVCALL(SD_BLE_GAP_CONN_PARAM_UPDATE, uint32_t, sd_ble_gap_conn_param_update(uint16_t conn_handle, ble_gap_conn_params_t const *p_conn_params));
int
nrf_gap_update_ppcp(uint16_t conn_id, const nest_connection_params_t *params)
{
  uint32_t err_code = NRF_SUCCESS;
  static ble_gap_conn_params_t conn_params;
  conn_params.min_conn_interval = params->min_conn_interval;
  conn_params.max_conn_interval = params->max_conn_interval;
  conn_params.slave_latency = params->slave_latency;
  conn_params.conn_sup_timeout = params->conn_sup_timeout;

  err_code = sd_ble_gap_conn_param_update(conn_id, &conn_params);
  if (err_code == NRF_SUCCESS) {
    PRINTF("[nest driver ppcp] update success\n");
    err_code = ENONE;
  } else if (err_code == NRF_ERROR_INVALID_ADDR) {
    PRINTF("[nest driver ppcp] update err:invalid addr\n");
    err_code = EFAULT;
  } else if (err_code == NRF_ERROR_INVALID_PARAM) {
    PRINTF("[nest driver ppcp] update err:invalid params\n");
    err_code = EINVAL;
  } else if (err_code == NRF_ERROR_INVALID_STATE) {
    PRINTF("[nest driver ppcp] update err:invalid state\n");
    err_code = EINVALSTATE;
  } else if (err_code == NRF_ERROR_BUSY) {
    PRINTF("[nest driver ppcp] update err:busy\n");
    err_code = EBUSY;
  } else if (err_code == BLE_ERROR_INVALID_CONN_HANDLE) {
    PRINTF("[nest driver ppcp] update err:invalid conn handle\n");
    err_code = EINVAL;
  } else if (err_code == NRF_ERROR_NO_MEM) {
    PRINTF("[nest driver ppcp] update err:no memory\n");
    err_code = ENOMEM;
  }
  return err_code;
}
/*---------------------------------------------------------------------------*/
