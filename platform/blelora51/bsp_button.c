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
#include "bsp_button.h"
#include <stdint.h>
#include <stdbool.h>
#include "frameworks/hw/hw_api.h"
#include "frameworks/hw/hw_api_null.h"
#include "frameworks/app_eventpool.h"
#include "frameworks/app_lifecycle.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "gpiote.h"
#include "sys/timer.h"
#include "errno.h"
#include "bsp_init.h"
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
PROCESS(bsp_button_process, "bsp btn process");

#define BUTTON_PRESS    	0x01
#define BUTTON_RELEASE    0x02

static struct timer debouncetimer;
static struct ctimer press_counter_ct;
static uint16_t m_indicator_long_press_ms = 2000;  // 2000 ms for long press
/*---------------------------------------------------------------------------*/
static void
duration_exceeded_callback(void *data)
{
	// PRINTF("[bsp button] duration exceeded callback\n");
  PRINTF("[bsp button] btn0 press\n");
	ctimer_stop(&press_counter_ct);
}
/*---------------------------------------------------------------------------*/
static void
btn_event_handler(gpiote_event_t *event)
{
	static int release = 0;
	uint32_t btn0_mask = (1u << BUTTON0);
	uint32_t data;

	if (event->pin_no == btn0_mask) {
		// PRINTF("[bsp button] btn %08x %08x %08x event\n", event->pin_no, event->pins_low_to_high_mask, event->pins_high_to_low_mask);
		if (event->pins_low_to_high_mask) {
			// PRINTF("[bsp button] btn %08x release\n", event->pin_no);
      data = BUTTON_RELEASE;
			if (process_is_running(&bsp_button_process))
				process_post(&bsp_button_process, PROCESS_EVENT_POLL, data);
		} else if (event->pins_high_to_low_mask) {
			// PRINTF("[bsp button] btn %08x press\n", event->pin_no);
      data = BUTTON_PRESS;
			if (process_is_running(&bsp_button_process))
				process_post(&bsp_button_process, PROCESS_EVENT_POLL, data);
		}
	}
}
/*---------------------------------------------------------------------------*/
static gpiote_handle_t gpioteh = {.event_handler=btn_event_handler,
								  .pins_mask = (1 << BUTTON0) /*| (1 << BUTTON1)*/,
								  .pins_low_to_high_mask=  (1 << BUTTON0) /*| (1 << BUTTON1)*/,
								  .pins_high_to_low_mask=  (1 << BUTTON0) /*| (1 << BUTTON1)*/,
								  .sense_high_pins = 0};
/*---------------------------------------------------------------------------*/
static void
app_terminating(void)
{
}
/*---------------------------------------------------------------------------*/
static struct app_lifecycle_event lifecycle_event = {
	.name = "hw_bsp_button",
	.terminating = app_terminating
};
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(bsp_button_process, ev, data)
{
	static uint32_t evt_type;
	static uint32_t notify_type;
	PROCESS_BEGIN();
	while (1) {
		PROCESS_WAIT_EVENT_UNTIL( (ev == PROCESS_EVENT_POLL));
		evt_type = data;
		if (evt_type == BUTTON_PRESS) {
			timer_set(&debouncetimer, m_indicator_long_press_ms);
			ctimer_set(&press_counter_ct, m_indicator_long_press_ms, duration_exceeded_callback, NULL);
		} else if (evt_type == BUTTON_RELEASE) {
			if(!timer_expired(&debouncetimer)) {
        PRINTF("[bsp button] btn0 click\n");
			} else {
        PRINTF("[bsp button] btn0 release\n");
			}
			ctimer_stop(&press_counter_ct);
		}
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
int
bsp_button_read(void *buf, uint32_t len, uint32_t offset)
{
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
bsp_button_subscribe(void *buf, uint32_t len, HWCallbackHandler handler)
{
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
bsp_button_init(void)
{
  nrf_gpio_cfg_input(BUTTON0, NRF_GPIO_PIN_NOPULL);
	app_lifecycle_register(&lifecycle_event);
	gpiote_register(&gpioteh);

  if (!process_is_running(&bsp_button_process))
    process_start(&bsp_button_process, NULL);
}
/*---------------------------------------------------------------------------*/
