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
#include "app_eventpool.h"
#include "app_framework.h"
#include "app_lifecycle.h"
#include "loader/symtab.h"
#include "errno.h"
#include "bsp_init.h"
#if defined(USE_FREERTOS)
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
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
#define mainAPPLICATION_INTERRUPT_TASK_PRIORITY         	  ( FREERTOS_APP_IRQ_TASK_PRIORITY_CONF )
extern TaskHandle_t m_application_main_task_handle;
extern SemaphoreHandle_t g_user_app_task_semaphore;

/*---------------------------------------------------------------------------*/
void
app_eventpool_context(void *arg)
{
  UNUSED_PARAMETER(arg);
  uint32_t ulNotificationValue;
  app_irq_event_t app_irq_event;

	for (;;) {
		if( xQueueReceive( g_app_irq_queue_handle, &( app_irq_event ), portMAX_DELAY) ) {
      // PRINTF("[eventpool task] event %d\n", app_irq_event.event_type);
      if (app_framework_is_app_running()) {
        switch (app_irq_event.event_type) {
          case APP_BLE_SCAN_EVENT:
          {
            app_irq_event.event_hook(&(app_irq_event.params.ble_scan_event));
          }
          break;
          case APP_BLE_ADV_EVENT:
          {
            app_irq_event.event_hook(&(app_irq_event.params.ble_adv_event));
          }
          break;
          case APP_BLE_CONN_EVENT:
          {
            app_irq_event.event_hook(&(app_irq_event.params.ble_conn_event));
          }
          break;
          case APP_BLE_CHARACTERISTIC_EVENT:
          {
            app_irq_event.event_hook(&(app_irq_event.params.ble_characteristic_event));
          }
          break;
          case APP_TIMER_EVENT:
          {
            app_irq_event.event_hook(&(app_irq_event.params.timer_event));
          }
          break;
          case APP_PROCESS_EVENT:
          {
            app_irq_event.event_hook(&(app_irq_event.params.process_event));
          }
          break;
          case APP_HW_UART_EVENT:
          {
            app_irq_event.event_hook(&(app_irq_event.params.hw_uart_event));
          }
          break;
          case APP_HW_GPIO_EVENT:
          {
            app_irq_event.event_hook(&(app_irq_event.params.hw_gpio_event));
          }
          break;
        }
        if( xSemaphoreGive( g_user_app_task_semaphore ) != pdTRUE ) {
        }

      }
    }
	}
	vTaskDelete( NULL);
}
/*---------------------------------------------------------------------------*/
int
app_eventpool_init(void)
{
  return ENONE;
}
/*---------------------------------------------------------------------------*/
