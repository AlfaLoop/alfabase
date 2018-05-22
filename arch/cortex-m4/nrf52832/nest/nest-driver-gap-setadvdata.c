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
static uint8_t m_adv_data[31];
static uint16_t m_adv_data_size;
static uint8_t m_scanrsp_adv_data[31];
static uint16_t m_scanrsp_adv_data_size;
/*
* @retval ::NRF_SUCCESS Advertising data successfully updated or cleared.
* @retval ::NRF_ERROR_INVALID_PARAM Invalid parameter(s) supplied, both p_data and p_sr_data cannot be NULL.
* @retval ::NRF_ERROR_INVALID_ADDR Invalid pointer supplied.
* @retval ::NRF_ERROR_INVALID_FLAGS Invalid combination of advertising flags supplied.
* @retval ::NRF_ERROR_INVALID_DATA Invalid data type(s) supplied, check the advertising data format specification.
* @retval ::NRF_ERROR_INVALID_LENGTH Invalid data length(s) supplied.
* @retval ::NRF_ERROR_NOT_SUPPORTED Unsupported data type.
* @retval ::BLE_ERROR_GAP_UUID_LIST_MISMATCH Invalid UUID list supplied.
*/
/*---------------------------------------------------------------------------*/
int
nrf_gap_set_adv_data(uint8_t *adv_data, uint16_t advsize, uint8_t *advsrp_data, uint16_t adv_scan_rsp_size)
{
	uint32_t err_code;
  if (adv_data == NULL) {
    return ENULLP;
  }

	PRINTF("[gap driver setadvdata] size %d scanrsp size %d\n", advsize, adv_scan_rsp_size);
	for (int i = 0; i < advsize; i++) {
		PRINTF("0x%02x ", adv_data[i]);
	}
	PRINTF("\n");

  if (advsize > 2 && adv_scan_rsp_size > 0) {
		memcpy(m_adv_data, adv_data, advsize);
		m_adv_data_size = advsize;
		memcpy(m_scanrsp_adv_data, advsrp_data, adv_scan_rsp_size);
		m_scanrsp_adv_data_size = adv_scan_rsp_size;
		err_code = sd_ble_gap_adv_data_set(m_adv_data, m_adv_data_size, m_scanrsp_adv_data, m_scanrsp_adv_data_size);
	} else if (advsize > 2) {
		memcpy(m_adv_data, adv_data, advsize);
		m_adv_data_size = advsize;
		err_code = sd_ble_gap_adv_data_set(m_adv_data, m_adv_data_size, NULL, 0);
	}

  if (err_code != NRF_SUCCESS) {
		PRINTF("[gap driver setadvdata] error %d\n", err_code);
		if (err_code == NRF_ERROR_NOT_SUPPORTED) {
		  return ENOSUPPORT;
	  } else {
			return EINVAL;
		}
	}
  return ENONE;
}
/*---------------------------------------------------------------------------*/
