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
#include <stdbool.h>
#include <string.h>
#include "gpiote.h"
#include "libs/util/linklist.h"
#include "nrf_error.h"
#include "nrf_gpio.h"
#include "app_util_platform.h"
/* Scheduler includes. */
#ifdef USE_FREERTOS
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
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
#define IRQ_PRIORITY_HIGH 3
LIST(gpiote_list);
// extern SemaphoreHandle_t g_contiki_task_semephore;
extern TaskHandle_t g_contiki_thread;


/**@brief Function for toggling sense level for specified pins.
 *
 * @param[in]   p_user   Pointer to user structure.
 * @param[in]   pins     Bitmask specifying for which pins the sense level is to be toggled.
 */
static void
sense_level_toggle(gpiote_handle_t *item, uint32_t pins)
{
  uint32_t pin_no;

  for (pin_no = 0; pin_no < NO_OF_PINS; pin_no++){
    uint32_t pin_mask = (1 << pin_no);

    if ((pins & pin_mask) != 0){
        uint32_t sense;

        // Invert sensing.
        if ((item->sense_high_pins & pin_mask) == 0){
            sense                    = GPIO_PIN_CNF_SENSE_High << GPIO_PIN_CNF_SENSE_Pos;
            item->sense_high_pins |= pin_mask;
        } else {
            sense                    = GPIO_PIN_CNF_SENSE_Low << GPIO_PIN_CNF_SENSE_Pos;
            item->sense_high_pins &= ~pin_mask;
        }

        NRF_GPIO->PIN_CNF[pin_no] &= ~GPIO_PIN_CNF_SENSE_Msk;
        NRF_GPIO->PIN_CNF[pin_no] |= sense;
    }
  }
}

/**@brief Function for sense disabling for all pins for specified user.
 *
 * @param[in]  user_id   User id.
 */
static void pins_sense_disable(gpiote_handle_t *handle)
{
  uint32_t pin_no;
	for (pin_no = 0; pin_no < 32; pin_no++){
		if ((handle->pins_mask & (1 << pin_no)) != 0)	{
			NRF_GPIO->PIN_CNF[pin_no] &= ~GPIO_PIN_CNF_SENSE_Msk;
			NRF_GPIO->PIN_CNF[pin_no] |= GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos;
		}
	}
}

/**@brief Function for handling the GPIOTE interrupt.
 */


/**@brief Function for handling the GPIOTE interrupt.
 */
void GPIOTE_IRQHandler(void)
{
  // static BaseType_t xHigherPriorityTaskWoken;
  BaseType_t yield_req = pdFALSE;

	static gpiote_handle_t *item = NULL;
	uint32_t transition_pins, pins_changed, late_transition_pins;
	uint32_t event_low_to_high, event_high_to_low;
	uint32_t pins_state = NRF_GPIO->IN;

  // Clear event.
  NRF_GPIOTE->EVENTS_PORT = 0;

  pins_state = NRF_GPIO->IN;
	PRINTF("[gpiote] pins_state: %08x \n", pins_state);

	// Check all register
  for (item = list_head(gpiote_list); item; item = item->next) {
    gpiote_event_t event;

		// Find set of pins on which there has been a transition.
		transition_pins = (pins_state ^ ~item->sense_high_pins) & item->pins_mask;
		PRINTF("[gpiote] transition_pins: %08x \n", transition_pins);

		// Toggle SENSE level for all pins that have changed state.
		sense_level_toggle(item, transition_pins);

		// Second read after setting sense.
		// Check if any pins have changed while serving this interrupt.
		pins_changed = NRF_GPIO->IN ^ pins_state;
		PRINTF("[gpiote] pins_changed: %08x \n", pins_changed);
		if (pins_changed)
		{
			pins_state          |= pins_changed;

			// Find set of pins on which there has been a transition.
			late_transition_pins = (pins_state ^ ~item->sense_high_pins) & item->pins_mask;

			// Toggle SENSE level for all pins that have changed state in last phase.
			sense_level_toggle(item, late_transition_pins);

			// Update pins that has changed state since the interrupt occurred.
			transition_pins |= late_transition_pins;
		}

		// Call user event handler if an event has occurred.
		event_high_to_low = (~pins_state & item->pins_high_to_low_mask) & transition_pins;
		event_low_to_high = (pins_state & item->pins_low_to_high_mask) & transition_pins;
		PRINTF("[gpiote] event_high_to_low: %08x \n", event_high_to_low);
		PRINTF("[gpiote] event_low_to_high: %08x \n", event_low_to_high);

		// call the callback for the specific irq
    event.pin_no = transition_pins;
    event.pins_low_to_high_mask = event_low_to_high;
    event.pins_high_to_low_mask = event_high_to_low;
		if ((event_low_to_high | event_high_to_low) != 0) {
      item->event_handler(&event);
		}
	}
#ifdef USE_FREERTOS
  vTaskNotifyGiveFromISR(g_contiki_thread, &yield_req);
  portYIELD_FROM_ISR(yield_req);
#endif
}
/*---------------------------------------------------------------------------*/
void
gpiote_init()
{
	list_init(gpiote_list);

	// Initialize GPIOTE interrupt (will not be enabled until gpiote_add() is called).
	NRF_GPIOTE->INTENCLR = 0xFFFFFFFF;

	NVIC_ClearPendingIRQ(GPIOTE_IRQn);
	NVIC_SetPriority(GPIOTE_IRQn, IRQ_PRIORITY_HIGH);
	NVIC_EnableIRQ(GPIOTE_IRQn);
}
/*---------------------------------------------------------------------------*/
void
gpiote_register(gpiote_handle_t *handle)
{
	uint32_t pin_no;
  uint32_t pins_state;

	if (handle->event_handler == NULL) {
		return;
	}

  // Clear any pending event.
  NRF_GPIOTE->EVENTS_PORT = 0;
  pins_state              = NRF_GPIO->IN;
	PRINTF("[gpiote] pins_mask: %08x \n", handle->pins_mask);

	if (!list_length(gpiote_list)) {
		// Make sure SENSE is disabled for all pins.
		pins_sense_disable(handle);
		NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_PORT_Msk;
	}

  // Enable sensing for all pins for specified user.
  handle->sense_high_pins = 0;
  for (pin_no = 0; pin_no < 32; pin_no++) {
    uint32_t pin_mask = (1 << pin_no);
    if ((handle->pins_mask & pin_mask) != 0) {
      uint32_t sense;
			if ((pins_state & pin_mask) != 0) {
        sense = GPIO_PIN_CNF_SENSE_Low << GPIO_PIN_CNF_SENSE_Pos;
      } else {
        sense = GPIO_PIN_CNF_SENSE_High << GPIO_PIN_CNF_SENSE_Pos;
        handle->sense_high_pins |= pin_mask;
      }
      NRF_GPIO->PIN_CNF[pin_no] &= ~GPIO_PIN_CNF_SENSE_Msk;
      NRF_GPIO->PIN_CNF[pin_no] |= sense;
    }
  }
	PRINTF("[gpiote] sense_high_pins: %08x \n", handle->sense_high_pins);
	list_add(gpiote_list, handle);
}
/*---------------------------------------------------------------------------*/
void
gpiote_unregister(gpiote_handle_t *handle)
{
	// Disable sensing for all pins for specified user.
  pins_sense_disable(handle);

	list_remove(gpiote_list, handle);

	if (!list_length(gpiote_list))
	{
		NRF_GPIOTE->INTENCLR = GPIOTE_INTENSET_PORT_Msk;
	}
}
/*---------------------------------------------------------------------------*/
