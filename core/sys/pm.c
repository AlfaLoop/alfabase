/**
 *  Copyright (c) 2016 AlfaLoop Technology Co., Ltd. All Rights Reserved.
 *
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential.
 *
 *  Attribution - You must give appropriate credit, provide a link to the license, and
 *  indicate if changes were made. You may do so in any reasonable manner, but not in any
 *  way that suggests the licensor endorses you or your use.
 *
 *  NonCommercial - You may not use the material for commercial purposes under unauthorized.
 *
 *  NoDerivatives - If you remix, transform, or build upon the material, you may not
 *  distribute the modified material.
 */
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
