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
#include "app_framework.h"
#include "app_eventpool.h"
#include "app_lifecycle.h"
#include "loader/elfloader.h"
#include "loader/lunchr.h"
#include "loader/symtab.h"
#include "loader/symbol-std.h"
#include "errno.h"

#if defined(USE_ELFLOADER)
#include "core/oslogger_api.h"
#include "core/ostimer_api.h"
#include "core/osfile_api.h"
#include "core/osprocess_api.h"
#include "core/osdevice_api.h"
#include "core/oslocaltime_api.h"
#include "hw/hw_api.h"
#include "ble/ble_api.h"
#endif


// FreeRTOS
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
#define mainAPPLICATION_MAIN_TASK_PRIORITY         	  		  ( FREERTOS_APP_TASK_PRIORITY_CONF )
#define mainAPPLICATION_INTERRUPT_TASK_PRIORITY         	  ( FREERTOS_APP_IRQ_TASK_PRIORITY_CONF )
#if defined(USE_ELFLOADER)
TaskHandle_t m_application_main_task_handle = NULL;
TaskHandle_t m_application_interrupt_task_handle = NULL;
extern SemaphoreHandle_t g_user_app_task_semaphore;

extern TaskHandle_t g_contiki_thread;
static OSProcessEntry_t m_app_entry = NULL;
static struct ctimer m_app_exit_ctimer;
#endif
/*---------------------------------------------------------------------------*/
#if defined(USE_ELFLOADER)
/*---------------------------------------------------------------------------*/
int
OSProcessEntrySetup(OSProcessEntry_t p_entry)
{
	PRINTF("[app framework] app_process entry setup\n");
	m_app_entry = p_entry;
	return app_framework_create_task();
}
static struct symbols symbolOSProcessEntrySetup = {
	.name = "OSProcessEntrySetup",
	.value = (void *)&OSProcessEntrySetup
};
/*---------------------------------------------------------------------------*/
static void
application_main_context(void *arg)
{
	UNUSED_PARAMETER(arg);
	PRINTF("[app framwork] App Task created \n");

	// contiki photothread exit, doesn't need it any more.
	autostart_exit(elfloader_autostart_processes);

	if (m_app_entry != NULL) {
		// start user's application
		m_app_entry();
		PRINTF("[app framwork] App Task exit!\n");
	}

  // remove entry pointer
	m_app_entry = NULL;

	osprocess_api_terminate();

  // release the syscall's resources
	app_lifecycle_notify(APP_LIFECYCLE_TERMINATE);

	m_application_main_task_handle = NULL;
	m_application_interrupt_task_handle = NULL;

	// lunchr_app_exit();

  // delete itself
	xQueueReset(g_app_irq_queue_handle);
	vTaskDelete(m_application_interrupt_task_handle);
	vTaskDelete( NULL);
}
#endif
/*---------------------------------------------------------------------------*/
#if defined(USE_ELFLOADER)
int
app_framework_create_task(void)
{
	if (m_application_main_task_handle != NULL ||
			m_application_interrupt_task_handle != NULL)
	{
		return EINVALSTATE;
	}

	if ( pdPASS != xTaskCreate( app_eventpool_context, "aec", LUNCHR_KERNEL_USER_APP_IRQ_TASK_SIZE,
          ( void * ) 0, mainAPPLICATION_INTERRUPT_TASK_PRIORITY,
          &m_application_interrupt_task_handle ) )
  {
    return ENOMEM;
  }

  if ( pdPASS != xTaskCreate( application_main_context, "amc", LUNCHR_KERNEL_USER_APP_TASK_SIZE,
          ( void * ) 0, mainAPPLICATION_MAIN_TASK_PRIORITY,
          &m_application_main_task_handle ) )
  {
    return ENOMEM;
  }

  return ENONE;
}
#endif
/*---------------------------------------------------------------------------*/
int
app_framework_remove_task(void)
{
	// restore resource used by app-po
#if defined(USE_ELFLOADER)
	if (m_application_main_task_handle == NULL ||
			m_application_interrupt_task_handle == NULL){
		return EINVALSTATE;
	}
#endif

	osprocess_api_terminate();

	// release the syscall resource
	app_lifecycle_notify(APP_LIFECYCLE_TERMINATE);

	// restore resource used by app-po
#if defined(USE_ELFLOADER)
	m_app_entry = NULL;
	vTaskDelete(m_application_main_task_handle);
	vTaskDelete(m_application_interrupt_task_handle);
	xQueueReset(g_app_irq_queue_handle);
	m_application_main_task_handle = NULL;
	m_application_interrupt_task_handle = NULL;
#endif

  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
app_framework_task_suspend(void)
{
	// uint32_t ulNotificationValue;
	if (m_application_main_task_handle == NULL) {
		return EINVALSTATE;
	}

	// do {
	// 	ulNotificationValue = ulTaskNotifyTake(pdTRUE,         /* Clear the notification value before exiting (equivalent to the binary semaphore). */
	// 											portMAX_DELAY); /* Block indefinitely (INCLUDE_vTaskSuspend has to be enabled).*/
	// 	// PRINTF("[main] ulNotificationValue %d\n", ulNotificationValue);
	// } while (ulNotificationValue == 4); // Timeout
	// PRINTF("[app framework] app task suspend\n");
	// vTaskSuspend(m_application_main_task_handle);
	// xTaskNotifyGive( g_contiki_thread );
	if( xSemaphoreTake( g_user_app_task_semaphore, portMAX_DELAY ) == pdTRUE ) {}
	return ENONE;
}
/*---------------------------------------------------------------------------*/
bool
app_framework_is_app_running(void)
{
	bool is_running = false;
#if defined(USE_ELFLOADER)
	if (m_app_entry == NULL) {
		is_running = false;
	} else {
		is_running = true;
	}
#endif
	return is_running;
}
/*---------------------------------------------------------------------------*/
int
app_framework_init(void)
{
	// Initialize lifecycle-control
	app_lifecycle_init();

	// Initialize dispatcher pool
	app_eventpool_init();

#if defined(USE_ELFLOADER)
  // Initialize symtab module
  symtab_init();
  symbol_std_init();

  // Add entry register syscall
  symtab_add(&symbolOSProcessEntrySetup);

	// Add all system API
	oslogger_api_init();
	osfile_api_init();
	ostimer_api_init();
	osdevice_api_init();
	oslocaltime_api_init();
	osprocess_api_init();
	ble_api_init();
	hw_api_init();

#endif  /* USE_ELFLOADER */

	PRINTF("[app framework] init completed\n");
}
/*---------------------------------------------------------------------------*/
