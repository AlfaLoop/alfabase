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
#include "pm.h"
#include "contiki.h"
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
static uint32_t current_mode = PM_NORMAL_MODE;
static uint8_t current_battery_level = 0;
/*---------------------------------------------------------------------------*/
PROCESS(pm_process, "pm_process");
/*---------------------------------------------------------------------------*/
uint8_t
pm_get_battery(void)
{
  return current_battery_level;
}
/*---------------------------------------------------------------------------*/
void
pm_switch_mode(uint32_t mode)
{
  current_mode = mode;
}
/*---------------------------------------------------------------------------*/
uint32_t
pm_current_mode(void)
{
  return current_mode;
}
/*---------------------------------------------------------------------------*/
void
pm_init(void)
{
  if (!process_is_running(&pm_process))
		process_start(&pm_process, NULL);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(pm_process, ev, data)
{
	static struct etimer st;
  static uint8_t mode;
	PROCESS_BEGIN();
  current_battery_level = PM.get_battery_level();
  mode = PM.get_charging_status();

	etimer_set(&st, CLOCK_SECOND * 30);
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&st) );
    current_battery_level = PM.get_battery_level();
    mode = PM.get_charging_status();
    if (mode == PM_SOURCE_BATTERY) {
      if (current_battery_level <= 0) {
        current_mode = PM_SAVING_MODE;
      } else {
        current_mode = PM_NORMAL_MODE;
      }
    } else if (mode == PM_SOURCE_CHARGING) {
      current_mode = PM_CHARGING_MODE;
    }
		etimer_reset(&st);
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
