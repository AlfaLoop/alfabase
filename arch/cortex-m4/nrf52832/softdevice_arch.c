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
#include "softdevice_arch.h"
#include <stdint.h>
#include <stdbool.h>
#include "nrf.h"
#include "nrf_soc.h"
#include "nrf_sdm.h"
#include "nrf_gpio.h"
#include "nrf_drv_clock.h"
#include "ble.h"
#include "ble_hci.h"
#include "nordic_common.h"
#include "softdevice_handler.h"
#include "gpiote.h"
#include "bsp_init.h"
#include "errno.h"
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
void
softdevice_init(void)
{
  uint32_t err_code;
  uint32_t retv;
  uint32_t app_ram_base;
  ble_enable_params_t params;

#if SD_CLOCK_SOURCE_CONF == true
  nrf_clock_lf_cfg_t clock_lf_cfg ={.source       = NRF_CLOCK_LF_SRC_XTAL,
               .rc_ctiv       = 0,
               .rc_temp_ctiv  = 0,
               .xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM};
#else
  nrf_clock_lf_cfg_t clock_lf_cfg ={.source       = NRF_CLOCK_LF_SRC_RC,
               .rc_ctiv       = 16,
               .rc_temp_ctiv  = 2,
               .xtal_accuracy = NULL};
#endif

  SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

  memset(&params, 0x00, sizeof(params));
#if NRF_SD_BLE_API_VERSION == 3
  params.common_enable_params.vs_uuid_count = NEST_ADD_GATTS_SERVICE_CONF;
  params.gap_enable_params.periph_conn_count = NEST_PERIPHERAL_LINK_COUNT_CONF;
  params.gap_enable_params.central_conn_count = NEST_CENTRAL_USER_LINK_COUNT_CONF;
  params.gap_enable_params.central_sec_count = 0;
  params.gatts_enable_params.service_changed = 1;
  params.gatts_enable_params.attr_tab_size = SD_BLE_GATTS_ATTR_TAB_SIZE_CONF;
  params.gatt_enable_params.att_mtu = SD_BLE_MAX_MTU_SIZE_CONF;
#elif NRF_SD_BLE_API_VERSION == 2
  params.common_enable_params.vs_uuid_count = NEST_ADD_GATTS_SERVICE_CONF;
  params.gap_enable_params.periph_conn_count = NEST_PERIPHERAL_LINK_COUNT_CONF;
  params.gap_enable_params.central_conn_count = NEST_CENTRAL_USER_LINK_COUNT_CONF;
  params.gap_enable_params.central_sec_count = 0;
  params.gatts_enable_params.service_changed = 1;
  params.gatts_enable_params.attr_tab_size = SD_BLE_GATTS_ATTR_TAB_SIZE_CONF;
#endif

  // set app_ram_base to the starting memory address of the application RAM,
	//retv = sd_ble_enable(&params, &app_ram_base);
	//PRINTF("app_ram_base: 0x%8x %d\n", app_ram_base, retv);
  //while(1){};
  PRINTF("[softdevice arch] softdevice_enable\n");
  err_code = softdevice_enable(&params);
  if (err_code != NRF_SUCCESS){
    PRINTF("[softdevice arch] enable error %d\n", err_code);
    return EINVAL;
  }
}
/*---------------------------------------------------------------------------*/
