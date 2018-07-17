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
#include "frameworks/hw/hw_api.h"
#include <stdint.h>
#include <stdbool.h>
#include "frameworks/app_eventpool.h"
#include "frameworks/app_lifecycle.h"
#include "gpiote.h"
#include "nrf_gpio.h"
#include "sys/clock.h"
#include "sys/ctimer.h"
#include "errno.h"
#include "bsp_init.h"
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
static PinEventHandler m_pin_event_handler = NULL;

static uint8_t *m_enable_pin_list;
static uint32_t m_enable_pin_size;
static bool m_interrupt_enable = false;
/*---------------------------------------------------------------------------*/
static void
hw_pin_irq_hooker(void *ptr)
{
	HwPinEvent *p_event = (HwPinEvent *)ptr;
	if (m_pin_event_handler != NULL) {
		m_pin_event_handler(p_event->pin, p_event->edge);
	}
}
/*---------------------------------------------------------------------------*/
static void
hw_pin_int_event_handler(gpiote_event_t *event)
{
  uint32_t pin = event->pin_no;
  uint32_t edge;

  if (event->pins_low_to_high_mask) {
    edge = PIN_EDGE_RISING;
  } else if (event->pins_high_to_low_mask) {
    edge = PIN_EDGE_FALLING;
  }

	if (m_pin_event_handler != NULL) {
		app_irq_event_t irq_event;
		irq_event.event_type = APP_HW_PIN_EVENT;
    irq_event.params.hwPinEvent.pin = pin;
    irq_event.params.hwPinEvent.edge = edge;
		irq_event.event_hook = hw_pin_irq_hooker;
		// push the timer callback event to user interrupt routin task
    xQueueSendFromISR( g_app_irq_queue_handle,  &irq_event, ( TickType_t ) 0 );
		// xQueueSend( g_app_irq_queue_handle,  &irq_event, ( TickType_t ) 0 );
	}
}
/*---------------------------------------------------------------------------*/
static gpiote_handle_t gpioteh = {.event_handler=hw_pin_int_event_handler,
								  .pins_mask = 0,
								  .pins_low_to_high_mask= 0,
								  .pins_high_to_low_mask= 0,
								  .sense_high_pins = 0 };
/*---------------------------------------------------------------------------*/
int
hw_pin_setup(uint32_t pin, uint8_t value)
{
  // PRINTF("[hw arch] pinSet %d value %d\n", pin, value);
  CHECK_PIN_ENABLED(pin);
  switch (value) {
    case PIN_DEFAULT:
      nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_NOPULL);
    break;
    case PIN_OUTPUT:
      nrf_gpio_cfg_output(pin);
      nrf_gpio_pin_clear(pin);
    break;
    case PIN_INPUT:
      nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_NOPULL);
    break;
    default:
      return EINVAL;
    break;
  }
	return ENONE;
}
/*---------------------------------------------------------------------------*/
int
hw_pin_output(uint32_t pin, uint8_t value)
{
  CHECK_PIN_ENABLED(pin);
  switch (value) {
    case PIN_OUTPUT_LOW:
        nrf_gpio_pin_clear(pin);
    break;
    case PIN_OUTPUT_HIGH:
      nrf_gpio_pin_set(pin);
    break;
		case PIN_OUTPUT_TOGGLE:
			nrf_gpio_pin_toggle(pin);
		break;
    default:
      return EINVAL;
    break;
  }
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
hw_pin_input(uint32_t pin, uint8_t value)
{
  CHECK_PIN_ENABLED(pin);
  switch (value) {
    case PIN_INPUT_FLOATING:
        nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_NOPULL);
    break;
    case PIN_INPUT_PULLUP:
      nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_PULLUP);
    break;
    case PIN_INPUT_PULLDOWN:
      nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_PULLDOWN);
    break;
    default:
      return EINVAL;
    break;
  }
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
hw_pin_read(uint32_t pin)
{
  CHECK_PIN_ENABLED(pin);
  return nrf_gpio_pin_read(pin);
}
/*---------------------------------------------------------------------------*/
int
hw_pin_attach_interrupt(uint32_t pin_number, uint32_t edge, PinEventHandler handler)
{
  uint32_t pins_mask = gpioteh.pins_mask;
  uint32_t rising_edge = gpioteh.pins_low_to_high_mask;
  uint32_t falling_edge = gpioteh.pins_high_to_low_mask;

  CHECK_PIN_ENABLED(pin_number);
  pins_mask = pins_mask | (gpioteh.pins_mask | (1U << pin_number));

  if (edge & PIN_EDGE_RISING) {
    rising_edge = rising_edge | (gpioteh.pins_mask | (1U << pin_number));
  }

  if (edge & PIN_EDGE_FALLING) {
    falling_edge = falling_edge | (gpioteh.pins_mask | (1U << pin_number));
  }

  gpioteh.pins_mask = pins_mask;
  gpioteh.pins_low_to_high_mask = rising_edge;
  gpioteh.pins_high_to_low_mask = falling_edge;

  gpiote_register(&gpioteh);
  m_interrupt_enable = true;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
hw_pin_detach_interrupt(void)
{
  gpiote_unregister(&gpioteh);
  m_interrupt_enable = false;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
Pin*
bsp_hw_pin_api_retrieve(void)
{
	static Pin instance;
  instance.setup = hw_pin_setup;
  instance.output = hw_pin_output;
  instance.input = hw_pin_input;
  instance.read = hw_pin_read;
  instance.attachInterrupt = hw_pin_attach_interrupt;
  instance.detachInterrupt = hw_pin_detach_interrupt;
  return &instance;
}
/*---------------------------------------------------------------------------*/
void
hw_pin_terminating(void)
{
  if (m_pin_event_handler != NULL)
    m_pin_event_handler = NULL;

  if (m_interrupt_enable) {
    gpiote_unregister(&gpioteh);
    m_interrupt_enable = false;
  }
}
/*---------------------------------------------------------------------------*/
