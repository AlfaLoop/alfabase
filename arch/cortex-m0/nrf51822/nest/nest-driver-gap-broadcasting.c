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
int
nrf_gap_broadcasting(nest_broadcast_params_t *params)
{
	uint32_t             		   err_code;
  ble_gap_adv_params_t 		   adv_params;
  uint32_t                   interval;

	if (params == NULL) {
		err_code = sd_ble_gap_adv_stop();
		if (err_code != NRF_SUCCESS) {
			// PRINTF("[nest broadcaster driver] adv stop error %d\n", err_code);
			return EINVALSTATE;
		}
		else
			return ENONE;
	}

	// Start advertising
	memset(&adv_params, 0, sizeof(adv_params));

	adv_params.type        = params->type;
	adv_params.p_peer_addr = NULL;
	adv_params.fp          = BLE_GAP_ADV_FP_ANY;
	adv_params.channel_mask.ch_37_off = 0;
	adv_params.channel_mask.ch_38_off = 0;
	adv_params.channel_mask.ch_39_off = 0;

	//  UNIT_0_625_MS convert
  interval = params->interval * 1000 / 625;
	if (params->type == BLE_GAP_ADV_TYPE_ADV_NONCONN_IND) {
    if (interval < BLE_GAP_ADV_NONCON_INTERVAL_MIN) {
      interval = BLE_GAP_ADV_NONCON_INTERVAL_MIN;
    }
  } else {
    if (interval < BLE_GAP_ADV_INTERVAL_MIN) {
      interval = BLE_GAP_ADV_INTERVAL_MIN;
    }
  }
	adv_params.interval    = interval;
	adv_params.timeout     = 0;

  uint8_t fs_processing = 0;
  do {
    fs_processing = is_fs_arch_processing();
  } while(fs_processing);

	err_code = sd_ble_gap_adv_start(&adv_params);
	if (err_code != NRF_SUCCESS) {
		// PRINTF("[nest broadcaster driver] adv start error %d\n", err_code);
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
			case NRF_ERROR_CONN_COUNT:
			// The limit of available connections has been reached;
			// connectable advertiser cannot be started.
			return ELOAR;
			break;
			case NRF_ERROR_NO_MEM:
			// The limit of available connections has been reached;
			// connectable advertiser cannot be started.
			return ENOMEM;
			break;
		}
		return EINVAL;
	}
	return ENONE;
}
/*---------------------------------------------------------------------------*/
