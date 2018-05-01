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
#include "contiki.h"
#include <string.h>
#include "errno.h"
#include "nest.h"
#include "dev/watchdog.h"

#ifdef USE_FRAMEWORK
#include "frameworks/ble/ble_api.h"
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
#define ERROR_RESET_MAX     4
#define RANDOM_TIMING			  4
#define DEFAULT_ACTIVE			1
#define DEFAULT_WINDOW			80
#define DEFAULT_MIN_WINDOW  20

/*---------------------------------------------------------------------------*/

static bool m_scanning = false;
/*---------------------------------------------------------------------------*/
// static void
// exithandler(void)
// {
//   PRINTF("[nest scanner] process exithandler\n");
//   NEST.gap_scan(NULL);
// }
#if defined(USE_FRAMEWORK)
/*---------------------------------------------------------------------------*/
PROCESS(nest_scan_api_process, "scanner process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_scan_api_process, ev, data)
{
	static nest_scanner_params_t params;
	static uint32_t ret;
  static uint16_t ble_scan_interval_level;
  static uint8_t error_count = 0;
  static struct etimer et;

	PROCESS_BEGIN();
	ble_scan_interval_level = ble_scan_api_interval_level();
	params.active = 1;
	params.interval = ble_scan_interval_level;
	params.window = ble_scan_interval_level;

	// connection mode
	// if (process_is_running(&nest_channel_process) || nest_is_peripheral_connected()) {
	// 	params.interval = DEFAULT_MIN_WINDOW;
	// 	params.window = DEFAULT_MIN_WINDOW;
	// }

	ret = NEST.gap_scan(&params);
	if (ret == ENONE) {
		// PRINTF("[nest scan api] scan api window %d\n", params.window);
    etimer_set(&et, params.window);
		m_scanning = true;
    PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&et) ||
                 ( ev == PROCESS_EVENT_EXIT ) ||
                 ( ev == nest_event_central_connected));
		m_scanning = false;
	 	NEST.gap_scan(NULL);
	 	etimer_stop(&et);
	 	error_count = 0;
	 	if ( (ev == PROCESS_EVENT_EXIT) ) {
			PRINTF("[nest scan api] process exit\n");
			ble_scan_completed(NEST_SCANNER_ABORT);
			process_exit(&nest_scan_api_process);
		}
  }  else {
    NEST.gap_scan(NULL);
    PRINTF("[nest scan api] nest scan failed %d\n", ret);
    // TODO: dirty code
    error_count++;
    if (error_count >= ERROR_RESET_MAX) {
      watchdog_reboot();
    }
  }
	// PRINTF("[nest scan api] ble scan completed\n");
	PROCESS_END();
}
#endif
/*---------------------------------------------------------------------------*/
PROCESS(nest_scan_central_process, "Nest scan channel");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_scan_central_process, ev, data)
{
	static nest_scanner_params_t params;
	static uint32_t ret;
	static uint8_t error_count = 0;
	static struct etimer et;

	PROCESS_BEGIN();

	params.active = 0;
	if (nest_is_peripheral_connected()) {
		params.interval = DEFAULT_MIN_WINDOW;
		params.window = DEFAULT_MIN_WINDOW;
	} else {
		params.interval = 100;
		params.window = 100;
	}
	ret = NEST.gap_scan(&params);
	if (ret == ENONE) {
		// PRINTF("[nest] scan channel window %d ms\n", params.window);
    etimer_set(&et, params.window);
		m_scanning = true;
    PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&et)  ||
									( ev == PROCESS_EVENT_EXIT ) ||
                 ( ev == nest_event_central_connected));
		NEST.gap_scan(NULL);
		m_scanning = false;
		etimer_stop(&et);
		error_count = 0;
		if ( ev == PROCESS_EVENT_EXIT ) {
			PRINTF("[nest scanner] process exit, admin found\n");
			process_exit(&nest_scan_central_process);
		}
  } else {
    NEST.gap_scan(NULL);
		PRINTF("[nest scanner] nest scan failed %d\n", ret);
    // TODO: dirty code
    error_count++;
    if (error_count >= ERROR_RESET_MAX) {
      watchdog_reboot();
    }
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
nest_scanner_init(void)
{
	nest_event_scanner_timeout = process_alloc_event();
}
/*---------------------------------------------------------------------------*/
bool
nest_scanner_scanning(void)
{
	return m_scanning;
}
/*---------------------------------------------------------------------------*/
