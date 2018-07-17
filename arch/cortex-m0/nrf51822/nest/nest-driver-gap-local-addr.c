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
nrf_getaddr(uint8_t *macaddr)
{
	uint32_t err_code = NRF_SUCCESS;
  ble_gap_addr_t  device_address;

  /* Get BLE address */
#if NRF_SD_BLE_API_VERSION == 3
	err_code = sd_ble_gap_addr_get(&device_address);
#elif NRF_SD_BLE_API_VERSION == 2
	err_code = sd_ble_gap_address_get(&device_address);
#endif

  if (err_code != NRF_SUCCESS) {
      return EFAULT;
  }

  memcpy(macaddr, device_address.addr, BLE_GAP_ADDR_LEN);
	return ENONE;
}
/*---------------------------------------------------------------------------*/
