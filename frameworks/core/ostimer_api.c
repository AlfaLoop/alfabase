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
#include "ostimer_api.h"
#include "contiki.h"
#include "frameworks/app_lifecycle.h"
#include "frameworks/app_eventpool.h"
#include "loader/symtab.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "errno.h"
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

// OS Timer resource
typedef struct{
	uint8_t used;
	uint16_t tick;
	uint8_t process;
	OSTimerHandler timer_handler_func;
} ostimer_instance_t;

static ostimer_instance_t m_ostimer_instances[OSTIMER_MAX_NUMBER];
extern TaskHandle_t g_contiki_thread;
/*---------------------------------------------------------------------------*/
static void
ostimer_irq_event_hooker(void *ptr)
{
	app_irq_timer_event_t *event = (app_irq_timer_event_t	*) ptr;
	uint8_t id = event->id;
	// PRINTF("[ostimer api] ostimer_irq_event_hooker id: %d\n", id);

	if (id >= OSTIMER_MAX_NUMBER || id < 0)
		return;

	if (m_ostimer_instances[id].timer_handler_func == NULL)
		return;

	// start call the function
	m_ostimer_instances[id].process = 1;
	m_ostimer_instances[id].timer_handler_func(id);
	m_ostimer_instances[id].process = 0;
}
/*---------------------------------------------------------------------------*/
static int
ostimer_start(TimerId id, uint16_t milliseconds, OSTimerHandler handler)
{
	uint32_t tick = clock_time_to_tick(milliseconds);

	if (id >= OSTIMER_MAX_NUMBER)
		return ENOMEM;

	if (handler == NULL)
		return EINVAL;

	m_ostimer_instances[id].used = 1;
	m_ostimer_instances[id].tick = tick;
	m_ostimer_instances[id].timer_handler_func = handler;

	if (id == 0)
		process_post(&ostimer_0_api, ostimer_event_timeout, NULL);
	else if (id == 1)
		process_post(&ostimer_1_api, ostimer_event_timeout, NULL);
	else if (id == 2)
		process_post(&ostimer_2_api, ostimer_event_timeout, NULL);

	xTaskNotifyGive( g_contiki_thread );
	taskYIELD();
	return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
ostimer_stop(TimerId id)
{
	if (id >= OSTIMER_MAX_NUMBER)
		return ENOMEM;

	m_ostimer_instances[id].used = 0;
	m_ostimer_instances[id].timer_handler_func = NULL;

	if (id == 0)
		process_post(&ostimer_0_api, ostimer_event_timeout, NULL);
	else if (id == 1)
		process_post(&ostimer_1_api, ostimer_event_timeout, NULL);
	else if (id == 2)
		process_post(&ostimer_2_api, ostimer_event_timeout, NULL);

	xTaskNotifyGive( g_contiki_thread );
	taskYIELD();
	return ENONE;
}
/*---------------------------------------------------------------------------*/
Timer*
OSTimer(void)
{
	static Timer instance;
	instance.start = ostimer_start;
	instance.stop = ostimer_stop;
	return &instance;
}
static struct symbols symbolOSTimer = {
	.name = "OSTimer",
	.value = (void *)&OSTimer
};
/*---------------------------------------------------------------------------*/
static void
app_terminating(void)
{
	// TODO: Add new API OS timer
	for (uint8_t i = 0; i < OSTIMER_MAX_NUMBER; i++) {
		m_ostimer_instances[i].used = 0;
		m_ostimer_instances[i].timer_handler_func = NULL;
		m_ostimer_instances[i].process = 0;
	}
}
/*---------------------------------------------------------------------------*/
static struct app_lifecycle_event lifecycle_event = {
	.name = "ostimer_api",
	.terminating = app_terminating
};
/*---------------------------------------------------------------------------*/
PROCESS(ostimer_0_api, "ostimer0_process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ostimer_0_api, ev, data)
{
	static struct etimer st;
	PROCESS_BEGIN();

	while(1) {
		PROCESS_WAIT_EVENT_UNTIL( (ev == ostimer_event_timeout) );
		// PRINTF("[ostimer0] start timer\n");
		if (m_ostimer_instances[0].used && m_ostimer_instances[0].timer_handler_func != NULL) {
			etimer_set(&st, m_ostimer_instances[0].tick);
			do {
				PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&st) );
				// PRINTF("[ostimer0] timer expired %d \n", clock_time());

				if (!m_ostimer_instances[0].process) {
					app_irq_event_t irq_event;
					irq_event.event_type = APP_TIMER_EVENT;
					irq_event.params.timer_event.id = 0;
					irq_event.event_hook = ostimer_irq_event_hooker;
					// push the timer callback event to user interrupt routin task
					xQueueSend( g_app_irq_queue_handle,  &irq_event, ( TickType_t ) 0 );
				}

				if (m_ostimer_instances[0].used && m_ostimer_instances[0].timer_handler_func != NULL) {
					// PRINTF("[ostimer0] reset timer0\n");
					etimer_reset(&st);
				}
			} while(m_ostimer_instances[0].used && m_ostimer_instances[0].timer_handler_func != NULL);

			etimer_stop(&st);
		}
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS(ostimer_1_api, "ostimer1_process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ostimer_1_api, ev, data)
{
	static struct etimer st;
	PROCESS_BEGIN();
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL( (ev == ostimer_event_timeout) );
		if (m_ostimer_instances[1].used && m_ostimer_instances[1].timer_handler_func != NULL) {
			etimer_set(&st, m_ostimer_instances[1].tick);
			do {
				PROCESS_WAIT_EVENT_UNTIL( timer_expired(&st) );

				if (m_ostimer_instances[1].process) {
					app_irq_event_t irq_event;
					irq_event.event_type = APP_TIMER_EVENT;
					irq_event.params.timer_event.id = 1;
					irq_event.event_hook = ostimer_irq_event_hooker;
					// push the timer callback event to user interrupt routin task
					xQueueSend( g_app_irq_queue_handle,  &irq_event, ( TickType_t ) 0 );
				}

				if (m_ostimer_instances[1].used && m_ostimer_instances[1].timer_handler_func != NULL) {
					etimer_reset(&st);
				}
			} while(m_ostimer_instances[1].used && m_ostimer_instances[1].timer_handler_func != NULL);
			etimer_stop(&st);
		}
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS(ostimer_2_api, "ostimer2_process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ostimer_2_api, ev, data)
{
	static struct etimer st;
	PROCESS_BEGIN();
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL( (ev == ostimer_event_timeout) );

		if (m_ostimer_instances[2].used && m_ostimer_instances[2].timer_handler_func != NULL) {
			etimer_set(&st, m_ostimer_instances[2].tick);
			do {
				PROCESS_WAIT_EVENT_UNTIL( timer_expired(&st) );

				if (m_ostimer_instances[2].process) {
					app_irq_event_t irq_event;
					irq_event.event_type = APP_TIMER_EVENT;
					irq_event.params.timer_event.id = 2;
					irq_event.event_hook = ostimer_irq_event_hooker;
					// push the timer callback event to user interrupt routin task
					xQueueSend( g_app_irq_queue_handle,  &irq_event, ( TickType_t ) 0 );
				}

				if (m_ostimer_instances[2].used && m_ostimer_instances[2].timer_handler_func != NULL) {
					etimer_reset(&st);
				}

			} while(m_ostimer_instances[2].used && m_ostimer_instances[2].timer_handler_func != NULL);

			etimer_stop(&st);
		}
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
ostimer_api_init(void)
{
	app_lifecycle_register(&lifecycle_event);

	memset(m_ostimer_instances, 0x00, OSTIMER_MAX_NUMBER * sizeof(ostimer_instance_t));
	symtab_add(&symbolOSTimer);

	ostimer_event_timeout = process_alloc_event();

	if (!process_is_running(&ostimer_0_api))
		process_start(&ostimer_0_api, NULL);
	if (!process_is_running(&ostimer_1_api))
		process_start(&ostimer_1_api, NULL);
	if (!process_is_running(&ostimer_2_api))
		process_start(&ostimer_2_api, NULL);
}
/*---------------------------------------------------------------------------*/
