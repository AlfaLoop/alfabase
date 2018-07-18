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
#include <string.h>
#include "errno.h"
#include "nest.h"
#include "dev/watchdog.h"

#ifdef USE_FRAMEWORK
#include "frameworks/ble/ble_api.h"
#endif
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
#define ERROR_RESET_MAX         4
#define RANDOM_TIMING			      4
#define DEFAULT_ACTIVE		     	1
#define DEFAULT_MAX_WINDOW			80
#define DEFAULT_MIN_WINDOW      20

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
	if ( (nest_central_status() != NEST_CENTRAL_STATUS_NONE) || nest_is_peripheral_connected()) {
		params.interval = DEFAULT_MIN_WINDOW;
		params.window = DEFAULT_MIN_WINDOW;
	}

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
		params.interval = DEFAULT_MAX_WINDOW;
		params.window = DEFAULT_MAX_WINDOW;
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
