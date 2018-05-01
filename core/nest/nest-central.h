/**
 *  Copyright (c) 2018 AlfaLoop Technology Co., Ltd. All Rights Reserved.
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
