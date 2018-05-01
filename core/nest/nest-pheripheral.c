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
#include "nest-pheripheral.h"
#include "frameworks/ble/ble_api.h"
#if defined(UED_WDUI_STACK)
#include "wdui/wdui.h"
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
PROCESS(nest_pheripheral_process, "Nest pheripheral");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_pheripheral_process, ev, data)
{
  static uint16_t conn_handle;
  PROCESS_BEGIN();

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL( (ev == nest_event_peripheral_connected ) ||
                              (ev == nest_event_peripheral_disconnect ) );
    if (ev == nest_event_peripheral_connected ) {
      conn_handle = data;
      ble_api_connection_event_handler(conn_handle, BLE_GATT_STATE_CONNECTED);
    } else if (ev == nest_event_peripheral_disconnect ) {
      conn_handle = data;
      ble_api_connection_event_handler(conn_handle, BLE_GATT_STATE_DISCONNECTED);
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
nest_pheripheral_connected_event(nest_context_t *ctx)
{
	uint16_t data = ctx->conn_id;
	if(process_is_running(&nest_pheripheral_process)) {
		process_post(&nest_pheripheral_process, nest_event_peripheral_connected, data);
	}
}
/*---------------------------------------------------------------------------*/
void
nest_pheripheral_disconnect_event(nest_context_t *ctx)
{
	uint16_t data = ctx->conn_id;
	if(process_is_running(&nest_pheripheral_process)) {
		process_post(&nest_pheripheral_process, nest_event_peripheral_disconnect, data);
	}
}
/*---------------------------------------------------------------------------*/
void
nest_pheripheral_init(void)
{
  if(!process_is_running(&nest_pheripheral_process))
		process_start(&nest_pheripheral_process, NULL);
}
/*---------------------------------------------------------------------------*/
