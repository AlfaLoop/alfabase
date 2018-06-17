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
