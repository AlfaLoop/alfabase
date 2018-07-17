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

/**@brief Start scanning (GAP Discovery procedure, Observer Procedure).
 *
 *
 * @events
 * @event{@ref BLE_GAP_EVT_ADV_REPORT, An advertising or scan response packet has been received.}
 * @event{@ref BLE_GAP_EVT_TIMEOUT, Scanner has timed out.}
 * @endevents
 *
 * @mscs
 * @mmsc{@ref BLE_GAP_SCAN_MSC}
 * @mmsc{@ref BLE_GAP_WL_SHARE_MSC}
 * @endmscs
 *
 * @param[in] p_scan_params Pointer to scan parameters structure.
 *
 * @retval ::NRF_SUCCESS Successfully initiated scanning procedure.
 * @retval ::NRF_ERROR_INVALID_ADDR Invalid pointer supplied.
 * @retval ::NRF_ERROR_INVALID_STATE Invalid state to perform operation.
 * @retval ::NRF_ERROR_INVALID_PARAM Invalid parameter(s) supplied.
 * @retval ::NRF_ERROR_BUSY The stack is busy, process pending events and retry.
 * @retval ::NRF_ERROR_RESOURCES Not enough BLE role slots available.
 *                               Stop one or more currently active roles (Central, Peripheral or Broadcaster) and try again
 */
// SVCALL(SD_BLE_GAP_SCAN_START, uint32_t, sd_ble_gap_scan_start(ble_gap_scan_params_t const *p_scan_params));
int
nrf_gap_scanner(nest_scanner_params_t *params)
{
	uint32_t              			err_code;
	ble_gap_scan_params_t       scan_param;
  uint32_t                    scan_window;
  uint32_t                    scan_interval;

	if (params == NULL) {
		err_code = sd_ble_gap_scan_stop();
		if (err_code != NRF_SUCCESS)
			return EINVALSTATE;
		else
			return ENONE;
	}
  // Scan window between 0x0004 and 0x4000 in 0.625ms units (2.5ms to 10.24s).
  // Scan interval between 0x0020 and 0x4000 in 0.625ms units (20ms to 10.24s).
	// No devices in whitelist, hence non selective performed.
	scan_param.active       = params->active;            // Active scanning set.
#if NRF_SD_BLE_API_VERSION == 3
	scan_param.use_whitelist    = 0;            // Selective scanning not set.
	scan_param.adv_dir_report  = 0;         // No whitelist provided.
#elif NRF_SD_BLE_API_VERSION == 2
	scan_param.selective   = 0;
	scan_param.p_whitelist = NULL;
#endif

  scan_interval = params->interval * 1000 / 625;
  scan_window = params->window * 1000 / 625;

	scan_param.interval     = scan_interval;//MSEC_TO_UNITS(params->interval, UNIT_0_625_MS);
	scan_param.window       = scan_window;//params->window;  // Scan window.
	scan_param.timeout      = 0x0000;       // Disable timeout

  // PRINTF("[nest driver] scan interval:%d window:%d\n", scan_interval, scan_window);
  err_code = sd_ble_gap_scan_start(&scan_param);
  if (err_code != NRF_SUCCESS) {
		// PRINTF("[nest driver] sd_ble_gap_scan_start error:%d \n", err_code );
		switch(err_code)
		{
			case NRF_ERROR_INVALID_ADDR:
			// Invalid pointer supplied.
			return EFAULT;
			break;
			case NRF_ERROR_INVALID_STATE:
			// Invalid state to perform operation
			return EINVALSTATE;
			break;
			case NRF_ERROR_INVALID_PARAM:
			// Invalid parameter(s) supplied
			return EINVAL;
			break;
			case NRF_ERROR_BUSY:
			// The stack is busy, process pending events and retry
			return EBUSY;
			break;
			case BLE_ERROR_GAP_WHITELIST_IN_USE:
			// Unable to replace the whitelist while another operation is using it
			return EBUSY;
			break;
			case NRF_ERROR_RESOURCES:
			// Not enough BLE role slots available.
			return ENOSUPPORT;
			break;
		}
	}

	return ENONE;
}
/*---------------------------------------------------------------------------*/
