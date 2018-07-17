/*
 * Copyright (c) Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 *   3. Neither the name of Nordic Semiconductor ASA nor the names of other
 *   contributors to this software may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 *   4. This software must only be used in a processor manufactured by Nordic
 *   Semiconductor ASA, or in a processor manufactured by a third party that
 *   is used in combination with a processor manufactured by Nordic Semiconductor.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
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
/**@brief Disconnect (GAP Link Termination).
 *
 * @details This call initiates the disconnection procedure, and its completion will be communicated to the application
 *          with a @ref BLE_GAP_EVT_DISCONNECTED event.
 *
 * @events
 * @event{@ref BLE_GAP_EVT_DISCONNECTED, Generated when disconnection procedure is complete.}
 * @endevents
 *
 * @mscs
 * @mmsc{@ref BLE_GAP_CONN_MSC}
 * @endmscs
 *
 * @param[in] conn_handle Connection handle.
 * @param[in] hci_status_code HCI status code, see @ref BLE_HCI_STATUS_CODES (accepted values are @ref BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION and @ref BLE_HCI_CONN_INTERVAL_UNACCEPTABLE).
 *
 * @retval ::NRF_SUCCESS The disconnection procedure has been started successfully.
 * @retval ::NRF_ERROR_INVALID_PARAM Invalid parameter(s) supplied.
 * @retval ::BLE_ERROR_INVALID_CONN_HANDLE Invalid connection handle supplied.
 * @retval ::NRF_ERROR_INVALID_STATE Invalid state to perform operation (disconnection is already in progress).
 */
/*---------------------------------------------------------------------------*/
int
nrf_gap_disconnect(uint16_t conn_id)
{
  uint32_t errcode;
  errcode = sd_ble_gap_disconnect(conn_id, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
  if (errcode == NRF_ERROR_INVALID_PARAM) {
    return EBADRQC;
  } else if (errcode == BLE_ERROR_INVALID_CONN_HANDLE){
    return EINVAL;
  } else if (errcode == NRF_ERROR_INVALID_STATE){
    return EINVALSTATE;
  }
  return ENONE;
}
/*---------------------------------------------------------------------------*/
