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
#include "nest-pheripheral.h"
#include "frameworks/ble/ble_api.h"
#if defined(UED_WDUI_STACK)
#include "wdui/wdui.h"
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
