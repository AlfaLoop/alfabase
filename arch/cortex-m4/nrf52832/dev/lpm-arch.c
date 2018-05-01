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
#include "dev/lpm.h"
#include "nrf.h"
#include "nrf_soc.h"
#include "nrf_sdm.h"
#include "ble.h"
#include "ble_hci.h"
#include "nordic_common.h"
#include "bsp_init.h"
#include "FreeRTOS.h"
#include "task.h"
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
// Set bit 7 and bits 4..0 in the mask to one (0x ...00 1001 1111)
 #define FPU_EXCEPTION_MASK 0x0000009F
/*---------------------------------------------------------------------------*/
void
lpm_drop(void)
{
	/* Clear exceptions and PendingIRQ from the FPU unit */
 	__set_FPSCR(__get_FPSCR()  & ~(FPU_EXCEPTION_MASK));
 	(void) __get_FPSCR();
 	NVIC_ClearPendingIRQ(FPU_IRQn);

#if defined(SOFTDEVICE_PRESENT)
	sd_app_evt_wait();
#else
	__WFE();
#endif
}
/*---------------------------------------------------------------------------*/
