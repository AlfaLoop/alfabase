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
#include "nest.h"
#include "errno.h"

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
#define NEST_BROADCASTER_SEND_TIMEOUT 		32
#if defined(USE_FRAMEWORK)
/*---------------------------------------------------------------------------*/
PROCESS(nest_broadcast_api_process, "broadcast api process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_broadcast_api_process, ev, data)
{
	static int err_code;
	static nest_broadcast_params_t adv_params;
	static struct etimer send_etimer;

	PROCESS_BEGIN();

	// Grab the adv mode and data
	ble_adv_api_params_retrieve(&adv_params);
	adv_params.interval = NEST_BROADCASTER_SEND_TIMEOUT;
	err_code = NEST.gap_broadcasting(&adv_params);
	if (err_code == ENONE) {
		etimer_set(&send_etimer, NEST_BROADCASTER_SEND_TIMEOUT);
		PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&send_etimer) );
		NEST.gap_broadcasting(NULL);
	} else {
		PRINTF("[nest broadcasting] error %i\n", err_code);
	}
	PROCESS_END();
}
#endif
/*---------------------------------------------------------------------------*/
void
nest_broadcaster_init(void)
{
	nest_event_broadcast_timeout = process_alloc_event();
}
/*---------------------------------------------------------------------------*/
