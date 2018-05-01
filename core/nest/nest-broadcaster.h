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
#ifndef _NEST_BROADCASTER_H_
#define _NEST_BROADCASTER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "nest.h"

enum {
	NEST_BROADCASTER_SENDOUT = 0x00
};

typedef struct {
	uint8_t  type;
	uint32_t interval;
} nest_broadcast_params_t;

/**
 * \brief      The event number for nest advertising timeout
 *
 *             The nest sends data as Contiki events to nest command
 *             processes. This variable contains the number of the
 *             Contiki event.
 *
 */
process_event_t nest_event_broadcast_timeout;
void nest_broadcaster_init(void);

PROCESS_NAME(nest_broadcast_api_process);

#ifdef __cplusplus
}
#endif
#endif /* _NEST_BROADCASTER_H_ */
