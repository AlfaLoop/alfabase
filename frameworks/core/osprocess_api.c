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
#include "osprocess_api.h"
#include "frameworks/app_lifecycle.h"
#include "frameworks/app_eventpool.h"
#include "loader/symtab.h"
#include "sys/clock.h"
#include "dev/logger.h"
#include "errno.h"
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
static OSProcessCallback m_process_callback;
static bool process_in_background = false;
/*---------------------------------------------------------------------------*/
static void
osprocess_irq_hooker(void *ptr)
{
  app_irq_process_event_t *p_event = (app_irq_process_event_t *) ptr;
  if (p_event->type == OSPROCESS_EVENT_TYPE_TERMINATE) {
    if (m_process_callback.onWillTerminate != NULL) {
      m_process_callback.onWillTerminate();
    }
  } else if (p_event->type == OSPROCESS_EVENT_TYPE_BACKGROUND) {
    if (m_process_callback.onEnterBackground != NULL) {
      m_process_callback.onEnterBackground();
    }
  } else if (p_event->type == OSPROCESS_EVENT_TYPE_FOREGROUND) {
    if (m_process_callback.onEnterForeground != NULL) {
      m_process_callback.onEnterForeground();
    }
  }
}
/*---------------------------------------------------------------------------*/
static int
osprocess_wait_for_event(void)
{
  return app_framework_task_suspend();
}
/*---------------------------------------------------------------------------*/
static uint32_t
osprocess_get_uuid(void)
{
  uint32_t uuid;
  if (lunchr_get_running_task_uuid(&uuid) == ENONE) {
    return ENONE;
  }
  return EINVALSTATE;
}
/*---------------------------------------------------------------------------*/
static uint32_t
osprocess_get_version(void)
{
  return ENOSUPPORT;
}
/*---------------------------------------------------------------------------*/
static uint32_t
osprocess_get_clock_tick(void)
{
  return clock_time();
}
/*---------------------------------------------------------------------------*/
static void
osprocess_delay(uint16_t millis)
{
  clock_wait(millis);
  return;
}
/*---------------------------------------------------------------------------*/
static void
osprocess_delayus(uint16_t microseconds)
{
  clock_delay_usec(microseconds);
  return;
}
/*---------------------------------------------------------------------------*/
Process*
OSProcess(void)
{
	static Process process;
	process.waitForEvent = osprocess_wait_for_event;
  process.getUuid = osprocess_get_uuid;
  process.getVersion = osprocess_get_version;
  process.getClockTick = osprocess_get_clock_tick;
  process.delay = osprocess_delay;
  process.delayUs = osprocess_delayus;
	return &process;
}
static struct symbols symbolOSProcess = {
	.name = "OSProcess",
	.value = (void *)&OSProcess
};
/*---------------------------------------------------------------------------*/
int
OSProcessEventAttach(const OSProcessCallback *callback)
{
  if (callback != NULL) {
   if (callback->onEnterBackground != NULL)
     m_process_callback.onEnterBackground = callback->onEnterBackground;
   if (callback->onEnterForeground != NULL)
     m_process_callback.onEnterForeground = callback->onEnterForeground;
   if (callback->onWillTerminate != NULL)
     m_process_callback.onWillTerminate = callback->onWillTerminate;
 } else {
   m_process_callback.onEnterBackground = NULL;
   m_process_callback.onEnterForeground = NULL;
   m_process_callback.onWillTerminate = NULL;
 }

 return ENONE;
}
static struct symbols symbolOSProcessEventAttach = {
	.name = "OSProcessEventAttach",
	.value = (void *)&OSProcessEventAttach
};
/*---------------------------------------------------------------------------*/
int
OSProcessEventDetach(void)
{
  m_process_callback.onEnterBackground = NULL;
  m_process_callback.onEnterForeground = NULL;
  m_process_callback.onWillTerminate = NULL;
  return ENONE;
}
static struct symbols symbolOSProcessEventDetach = {
	.name = "OSProcessEventDetach",
	.value = (void *)&OSProcessEventDetach
};
/*---------------------------------------------------------------------------*/
static void
app_terminating(void)
{
  m_process_callback.onEnterBackground = NULL;
  m_process_callback.onEnterForeground = NULL;
  m_process_callback.onWillTerminate = NULL;
  process_in_background = false;
}
/*---------------------------------------------------------------------------*/
static struct app_lifecycle_event lifecycle_event = {
	.name = "osprocess",
	.terminating = app_terminating
};
/*---------------------------------------------------------------------------*/
void
osprocess_api_init(void)
{
	symtab_add(&symbolOSProcess);
  symtab_add(&symbolOSProcessEventAttach);
  symtab_add(&symbolOSProcessEventDetach);

  app_lifecycle_register(&lifecycle_event);

  m_process_callback.onEnterBackground = NULL;
  m_process_callback.onEnterForeground = NULL;
  m_process_callback.onWillTerminate = NULL;
}
/*---------------------------------------------------------------------------*/
void
osprocess_api_terminate(void)
{
  process_in_background = false;
  if (m_process_callback.onWillTerminate != NULL) {
    app_irq_event_t irq_event;
    irq_event.event_type = APP_PROCESS_EVENT;
    irq_event.params.process_event.type = OSPROCESS_EVENT_TYPE_TERMINATE;
    irq_event.event_hook = osprocess_irq_hooker;
    xQueueSend( g_app_irq_queue_handle,  &irq_event, ( TickType_t ) 0 );
  }
}
/*---------------------------------------------------------------------------*/
void
osprocess_api_background(void)
{
  process_in_background = true;
  if (m_process_callback.onEnterForeground != NULL) {
    app_irq_event_t irq_event;
    irq_event.event_type = APP_PROCESS_EVENT;
    irq_event.params.process_event.type = OSPROCESS_EVENT_TYPE_FOREGROUND;
    irq_event.event_hook = osprocess_irq_hooker;
    xQueueSend( g_app_irq_queue_handle,  &irq_event, ( TickType_t ) 0 );
  }
}
/*---------------------------------------------------------------------------*/
void
osprocess_api_foreground(void)
{
  process_in_background = false;
  if (m_process_callback.onEnterBackground != NULL) {
    app_irq_event_t irq_event;
    irq_event.event_type = APP_PROCESS_EVENT;
    irq_event.params.process_event.type = OSPROCESS_EVENT_TYPE_BACKGROUND;
    irq_event.event_hook = osprocess_irq_hooker;
    xQueueSend( g_app_irq_queue_handle,  &irq_event, ( TickType_t ) 0 );
  }
}
/*---------------------------------------------------------------------------*/
bool
osprocess_app_in_background(void)
{
  return process_in_background;
}
/*---------------------------------------------------------------------------*/
