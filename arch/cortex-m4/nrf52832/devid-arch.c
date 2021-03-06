/* Copyright (c) 2016, Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *   * Neither the name of Nordic Semiconductor ASA nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
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
#include "nrf.h"
#include "nrf_soc.h"
#include "sys/devid.h"
#include "sys/bootloader.h"
#include "sys/pm.h"
#include "spiffs/spiffs.h"
#include "dev/watchdog.h"
#include "nordic_common.h"
#include "nrf_mbr.h"
#include "nrf_sdm.h"
#include "errno.h"
#include "nest/nest.h"
#include "nrf_gpio.h"
#include "app_util.h"
#include "app_error.h"
#include "softdevice_handler.h"
#include "ble_stack_handler_types.h"
#include "ble_advdata.h"
#include "ble_l2cap.h"
#include "ble_gap.h"
#include "ble_gatt.h"
#include "ble_hci.h"
#include "app_util.h"

#ifdef USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#endif

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

#ifndef min
#define min(a, b) ((a)<(b)?(a):(b))
#endif
#define CODE_PAGE_SIZE                      0x1000                      /**< Size of a flash codepage. Used for size of the reserved flash space in the bootloader region. Will be runtime checked against NRF_UICR->CODEPAGESIZE to ensure the region is correct. */

#define DFU_MAGIC_NUM									  0x00000044

#define BOOTLOADER_REGION_START         0x00078000
#define BOOTLOADER_REGION_SIZE          0x00007000
#define BOOTLOADER_SETTINGS_ADDRESS     0x0007F000
#define BOOTLOADER_SETTINGS_SIZE     		0x00001000
#define CODE_REGION_1_START						  0x0001F000

#define IRQ_ENABLED             			 0x01                    								/**< Field identifying if an interrupt is enabled. */
#define MAX_NUMBER_INTERRUPTS   			 32                      								/**< Maximum number of interrupts available. */

#define DFU_RESET_TIMEOUT 						 500
/*---------------------------------------------------------------------------*/
/*
 * These values are generated at random.
 * DEVICEID[0-1] and DEVICEADDR[0-1].
 */
int
devid_gen(uint8_t *id, int max_len)
{
  int len, cnt;

  cnt = min(sizeof(NRF_FICR->DEVICEID), max_len);
  memcpy(id, (void *)NRF_FICR->DEVICEID, cnt);
  len = cnt;

  cnt = min(sizeof(NRF_FICR->DEVICEADDR), max_len - len);
  memcpy(id + len, (void *)NRF_FICR->DEVICEADDR, cnt);

  return len + cnt;
}
/*---------------------------------------------------------------------------*/
int
hwid_gen(uint8_t *id, int max_len)
{
	int len, cnt;

	cnt = min(sizeof(NRF_FICR->DEVICEADDR), max_len);
	memcpy(id, (void *)NRF_FICR->DEVICEADDR, cnt);
	len = cnt;

	return len;
}
/*---------------------------------------------------------------------------*/
int
devid_reboot(void)
{
  NVIC_SystemReset();
}
/*---------------------------------------------------------------------------*/
int
devid_bootloader_mode(void)
{
	uint32_t err_code;
  // static bootloader_settings_t settings;
	// change bank state to SWITCH
  // bootloader_settings_get(&settings);
	// settings.bank = BANK_SWITCH;
	//
	// PRINTF("[devid-arch] settings.bank %d\n", settings.bank);
	// PRINTF("[devid-arch] settings.bank_checksum 0x%08x\n", settings.bank_signature);
	// PRINTF("[devid-arch] settings.bank_size %d\n", settings.bank_signature_size);
	// PRINTF("[devid-arch] settings.counter_low %d\n", settings.counter_low);
	// PRINTF("[devid-arch] settings.counter_high %d\n", settings.counter_high);
	// bootloader_settings_save(&settings);
	// Switch to bootloader mode and reset the system
#if NRF_SD_BLE_API_VERSION == 3
#elif NRF_SD_BLE_API_VERSION == 2
#endif

	// Switch to bootloader mode and reset the system
#if NRF_SD_BLE_API_VERSION == 3
	err_code = sd_power_gpregret_clr(0, 0xffffffff);
	PRINTF("[devid-arch] sd_power_gpregret_clr %d\n", err_code);
	//err_code = sd_power_gpregret_clr(0xFF);
	err_code = sd_power_gpregret_set(0, DFU_MAGIC_NUM);
	PRINTF("[devid-arch] sd_power_gpregret_set %d\n", err_code);
#elif NRF_SD_BLE_API_VERSION == 2
	err_code = sd_power_gpregret_clr(0xffffffff);
	PRINTF("[devid-arch] sd_power_gpregret_clr %d\n", err_code);
	//err_code = sd_power_gpregret_clr(0xFF);
	err_code = sd_power_gpregret_set(DFU_MAGIC_NUM);
	PRINTF("[devid-arch] sd_power_gpregret_set %d\n", err_code);
#endif

	pm_switch_mode(PM_SAVING_MODE);
	bootloader_start();
	return ENONE;
}
/*---------------------------------------------------------------------------*/
