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
#include "hardfault.h"
#include "spiffs/spiffs.h"
#include "loader/lunchr.h"
#include "nest/nest.h"
#include "bsp_init.h"

#ifdef USE_FRAMEWORK
#include "frameworks/app_framework.h"
#include "frameworks/core/oslogger_api.h"
#endif

#if defined(USE_WDUI_STACK)
#include "wdui/wdui.h"
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
PROCESS(hardfault_process, "hardfault process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hardfault_process, ev, data)
{
	static hardfault_dump_t *dump;
	PROCESS_BEGIN();
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL( (ev == PROCESS_EVENT_CONTINUE) );
		dump = data;
		PRINTF ("\n[hardfault] type %d\n", dump->type);
	  PRINTF ("[hardfault] R0 = %x\n", dump->r0);
	  PRINTF ("[hardfault] R1 = %x\n", dump->r1);
	  PRINTF ("[hardfault] R2 = %x\n", dump->r2);
	  PRINTF ("[hardfault] R3 = %x\n", dump->r3);
	  PRINTF ("[hardfault] R12 = %x\n", dump->r12);
	  PRINTF ("[hardfault] LR [R14] = %x  subroutine call return address\n", dump->lr);
	  PRINTF ("[hardfault] PC [R15] = %x  program counter\n", dump->pc);
	  PRINTF ("[hardfault] PSR = %x\n", dump->psr);

#if defined(USE_FRAMEWORK)
    if (lunchr_is_running()) {

#if SYS_HARDFAULT_KILL_BOOT_APP == 1
      lunchr_remove_boot_task();
      lunchr_kill_running_app();
#else
      devid_reboot();
#endif

#if defined(USE_WDUI_STACK)
		wdui_nest_event_notify(NEST_LUNCHR_KILL, NULL);
#endif
      // report error
      if (dump->type == HADRFAULT_DIV_0) {
        oslog_printf(LOG_BLE | LOG_SERIAL | LOG_RTT, "\nHardfault: Divide by zero\n");
      } else if (dump->type == HADRFAULT_UNALIGNED_ACCESS) {
        oslog_printf(LOG_BLE | LOG_SERIAL | LOG_RTT, "\nHardfault: Unaligned access\n");
      } else if (dump->type == HADRFAULT_NO_COPROCESSOR) {
        oslog_printf(LOG_BLE | LOG_SERIAL | LOG_RTT, "\nHardfault: No coprocessor\n");
      } else if (dump->type == HADRFAULT_INVALID_PC_LOAD) {
        oslog_printf(LOG_BLE | LOG_SERIAL | LOG_RTT, "\nHardfault: Invalid PC load\n");
      } else if (dump->type == HADRFAULT_INVALID_STATE) {
        oslog_printf(LOG_BLE | LOG_SERIAL | LOG_RTT, "\nHardfault: Invalid state\n");
      } else if (dump->type == HADRFAULT_UNDEFINED_INSTRUCTION) {
        oslog_printf(LOG_BLE | LOG_SERIAL | LOG_RTT, "\nHardfault: Undefined instruction\n");
      } else if (dump->type == HADRFAULT_BUS) {
        oslog_printf(LOG_BLE | LOG_SERIAL | LOG_RTT, "\nHardfault: Bus error\n");
      } else if (dump->type == HADRFAULT_MEMORT_MANAGEMENT) {
        oslog_printf(LOG_BLE | LOG_SERIAL | LOG_RTT, "\nHardfault: MemoryManagement error\n");
      } else {
        oslog_printf(LOG_BLE | LOG_SERIAL | LOG_RTT, "\nHardfault: Undefined instruction\n");
      }
    } else {
			devid_reboot();
		}
#endif /* USE_FRAMEWORK */
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
