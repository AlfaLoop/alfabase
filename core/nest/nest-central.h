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
#ifndef _NEST_CENTRAL_H_
#define _NEST_CENTRAL_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "nest.h"

enum {
  NEST_CENTRAL_STATUS_NONE = 0x00,
	NEST_CENTRAL_STATUS_INITIATING = 0x01,
	NEST_CENTRAL_STATUS_SERVICE_DISCOVERY = 0x02,
	NEST_CENTRAL_STATUS_ENABLE_NOTIFICATION = 0x03,
	NEST_CENTRAL_STATUS_PROCESS_COMMAND = 0x04,
};

void nest_central_init(void);
void nest_central_initiate_connection(uint8_t *peer_addr, uint8_t peer_type);
void nest_central_connected_event(uint16_t conn_id, uint8_t *address, uint8_t type);
void nest_central_disconnect_event(uint16_t conn_id, uint8_t reason);
void nest_central_gattc_handle_value_event(uint16_t conn_id, uint16_t handle,
      const uint8_t *data, uint16_t length);

uint8_t nest_central_adv_filter(uint8_t *src, uint8_t length);
uint8_t nest_central_status(void);

PROCESS_NAME(nest_central_send_process);

#ifdef __cplusplus
}
#endif
#endif /* _NEST_CENTRAL_H_ */
