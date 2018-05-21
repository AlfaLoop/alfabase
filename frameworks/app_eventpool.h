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
#ifndef _APP_EVENTPOOL_H_
#define _APP_EVENTPOOL_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "frameworks/core/ostimer_api.h"
#include "frameworks/core/osprocess_api.h"
#include "frameworks/ble/ble_api.h"
#include "frameworks/hw/hw_api.h"

typedef enum {
	APP_BLE_SCAN_EVENT = 0x00,
	APP_BLE_ADV_EVENT = 0x01,
	APP_BLE_CONN_EVENT = 0x02,
	APP_BLE_CHARACTERISTIC_EVENT = 0x03,
	APP_TIMER_EVENT,
	APP_PROCESS_EVENT,
	APP_HW_EVENT
} app_irq_event_type_t;

typedef struct {
  uint16_t conn_handle;
  uint16_t state;
} app_irq_ble_conn_event_t;

typedef struct {
	uint8_t 		type;
	ScanRecord 	record;
} app_irq_ble_scan_event_t;

typedef struct {
	uint8_t 		type;
} app_irq_ble_adv_event_t;


typedef struct {
  uint8_t  type;
  uint16_t conn_handle;
  uint16_t handle;
  uint8_t  value[20];
  uint16_t length;
} app_irq_ble_characteristic_event_t;

typedef struct{
  uint8_t id;
} app_irq_timer_event_t;

typedef struct {
  uint32_t type;
} app_irq_process_event_t;

typedef struct{
	void *params ;
} app_irq_hw_event_t;

typedef struct {
	uint8_t event_type;
	union {
		app_irq_ble_scan_event_t 					  ble_scan_event;
		app_irq_ble_adv_event_t 					  ble_adv_event;
		app_irq_ble_conn_event_t 				    ble_conn_event;
		app_irq_ble_characteristic_event_t	ble_characteristic_event;
		app_irq_timer_event_t							  timer_event;
		app_irq_process_event_t						  process_event;
		app_irq_hw_event_t						      hw_event;
	} params;
	void   (* event_hook)(void *ptr);
} app_irq_event_t;

// Global variable
QueueHandle_t g_app_irq_queue_handle;
void app_eventpool_context(void *arg);

#ifdef __cplusplus
}
#endif
#endif /* _APP_EVENTPOOL_H_ */
