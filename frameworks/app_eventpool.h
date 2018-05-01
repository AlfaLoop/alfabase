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
	APP_SENSOR_EVENT,
	APP_TIMER_EVENT,
	APP_UIKIT_EVENT,
	APP_PROCESS_EVENT,
	APP_HW_UART_EVENT,
	APP_HW_PIN_EVENT
} app_irq_event_type_t;

typedef struct {
	uint8_t 		type;
	ScanRecord 	record;
} app_irq_ble_scan_event_t;

typedef struct {
	uint8_t 		type;
} app_irq_ble_adv_event_t;

typedef struct{
  uint8_t id;
} app_irq_timer_event_t;

typedef struct{
	uint8_t data;
} app_irq_hw_uart_event_t;

typedef struct {
	uint8_t event_type;
	union {
		app_irq_ble_scan_event_t 					ble_scan_event;
		app_irq_ble_adv_event_t 					ble_adv_event;
		BleConnEvent 											bleConnEvent;
		BleCharacteristicEvent						bleCharacteristicEvent;
		app_irq_timer_event_t							timer_event;
		OSProcessEvent										processEvent;
		app_irq_hw_uart_event_t						hw_uart_event;
		HwPinEvent												hwPinEvent;
	} params;
	void   (* event_hook)(void *ptr);
} app_irq_event_t;


// Global variable
QueueHandle_t g_app_irq_queue_handle;

void app_eventpool_context(void *arg);
// void app_eventpool_wait(void);
// void app_eventpool_wait_arch(void);

#ifdef __cplusplus
}
#endif
#endif /* _APP_EVENTPOOL_H_ */
