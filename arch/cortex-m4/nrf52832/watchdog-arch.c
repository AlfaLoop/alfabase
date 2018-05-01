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
#include "nrf_sdm.h"
#include "nrf_gpio.h"
#include "bsp/drivers/nrf_drv_wdt.h"
#include "sdk_config.h"
#include "dev/watchdog.h"
#include "bsp_init.h"
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
static nrf_drv_wdt_channel_id m_channel_id;
static uint8_t  m_wdt_enable = 0;
/*---------------------------------------------------------------------------*/
void
wdt_event_handler(void)
{
	PRINTF("[watchdog arch] event occur\n");
}
/*---------------------------------------------------------------------------*/
void
watchdog_init(void)
{
	PRINTF("[watchdog-arch] init\n");
	uint32_t err_code;
	nrf_drv_wdt_config_t config =  {
        .behaviour          = NRF_WDT_BEHAVIOUR_RUN_SLEEP,
        .reload_value       = 4000,
        .interrupt_priority = 6,
    };
  err_code = nrf_drv_wdt_init(&config, wdt_event_handler);
	PRINTF("[watchdog arch] nrf_drv_wdt_init ret: %d\n", err_code);
}
/*---------------------------------------------------------------------------*/
void
watchdog_start(void)
{
  nrf_drv_wdt_channel_alloc(&m_channel_id);
  nrf_drv_wdt_enable();
	m_wdt_enable = 1;
}
/*---------------------------------------------------------------------------*/
void
watchdog_periodic(void)
{
	if (m_wdt_enable) {
		nrf_drv_wdt_channel_feed(m_channel_id);
		//nrf_drv_wdt_feed();
	}
}
/*---------------------------------------------------------------------------*/
void
watchdog_stop(void)
{
	nrf_wdt_int_disable(NRF_WDT_INT_TIMEOUT_MASK);
}
/*---------------------------------------------------------------------------*/
void
watchdog_reboot(void)
{
	m_wdt_enable = 0;
}
/*---------------------------------------------------------------------------*/
