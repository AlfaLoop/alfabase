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
