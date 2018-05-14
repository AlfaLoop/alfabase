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
#include "frameworks/hw/hw_api.h"
#include <stdint.h>
#include <stdbool.h>
#include "frameworks/app_eventpool.h"
#include "frameworks/app_lifecycle.h"
#include "gpiote.h"
#include "nrf_gpio.h"
#include "sys/clock.h"
#include "dev/uart.h"
#include "dev/spi.h"
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

#define CHECK_UART_ENABLED(pin)                                   \
   do                                                             \
   {                                                              \
      if (!hw_api_check_pin(pin, HW_PIN_UART))                    \
        return ENOSUPPORT;                                        \
   } while (0)


#define CHECK_PIN_ENABLED(pin)                                    \
   do                                                             \
   {                                                              \
      if (!hw_api_check_pin(pin, HW_PIN_GPIO))                    \
        return ENOSUPPORT;                                        \
   } while (0)

static PinEventHandler m_pin_event_handler = NULL;
static uint8_t *m_enable_pin_list;
static uint32_t m_enable_pin_size;
static bool m_interrupt_enable = false;

extern const struct uart_driver uart0;
static UartRxHandler m_uart_handler = NULL;

/*---------------------------------------------------------------------------*/
static void
hw_uart_rx_irq_hooker(void *ptr)
{
	app_irq_hw_uart_event_t *p_event = (app_irq_hw_uart_event_t *)ptr;
	if (m_uart_handler != NULL) {
		m_uart_handler(p_event->data);
	}
}
/*---------------------------------------------------------------------------*/
static void
uart_rx_handler(uint8_t data)
{
  PRINTF("[bps_init] rx data: %02x\n", data);
	if (m_uart_handler != NULL) {
		app_irq_event_t irq_event;
		irq_event.event_type = APP_HW_UART_EVENT;
		irq_event.params.hw_uart_event.data = data;
		irq_event.event_hook = hw_uart_rx_irq_hooker;
		//xQueueSend( g_app_irq_queue_handle,  &irq_event, ( TickType_t ) 0 );
    xQueueSendFromISR( g_app_irq_queue_handle,  &irq_event, ( TickType_t ) 0 );
	}
}
/*---------------------------------------------------------------------------*/
static uart_config_t m_uart0_arch_config = {
	.tx = 0,
	.rx = 0,
	.cts = 0,
	.rts = 0,
	.baudrate = UART_BAUDRATE_BAUDRATE_Baud9600,
	.hwfc = UART_HWFC,
	.cb = uart_rx_handler
};
/*---------------------------------------------------------------------------*/
static int
hw_api_uart0_open(uint32_t pin_tx, uint32_t pin_rx, uint32_t baudrate, UartRxHandler handler)
{
  CHECK_PIN_ENABLED(pin_tx);
  CHECK_PIN_ENABLED(pin_rx);

  uint32_t nrf_baudrate = 0;
  // update the baudrate
  switch (baudrate)
  {
    case BAUDRATE_1200:
    nrf_baudrate = UART_BAUDRATE_BAUDRATE_Baud1200;
    break;
    case BAUDRATE_2400:
    nrf_baudrate = UART_BAUDRATE_BAUDRATE_Baud2400;
    break;
    case BAUDRATE_4800:
    nrf_baudrate = UART_BAUDRATE_BAUDRATE_Baud4800;
    break;
    case BAUDRATE_9600:
    nrf_baudrate = UART_BAUDRATE_BAUDRATE_Baud9600;
    break;
    case BAUDRATE_19200:
    nrf_baudrate = UART_BAUDRATE_BAUDRATE_Baud19200;
    break;
    case BAUDRATE_38400:
    nrf_baudrate = UART_BAUDRATE_BAUDRATE_Baud38400;
    break;
    case BAUDRATE_57600:
    nrf_baudrate = UART_BAUDRATE_BAUDRATE_Baud57600;
    break;
    case BAUDRATE_115200:
    nrf_baudrate = UART_BAUDRATE_BAUDRATE_Baud115200;
    break;
    default:
      return EINVAL;
    break;
  }

  m_uart0_arch_config.tx = pin_tx;
  m_uart0_arch_config.rx = pin_rx;
  m_uart0_arch_config.baudrate = nrf_baudrate;

  PRINTF("[bsp_init] UART: %d %d %d\n", pin_tx, pin_rx, nrf_baudrate);
  if (handler != NULL)
    m_uart_handler = handler;
  else
    m_uart_handler = NULL;

  uart0.init(&m_uart0_arch_config);
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
hw_api_uart0_close(void)
{
  uart0.disable();
}
/*---------------------------------------------------------------------------*/
static int
hw_api_uart0_send(uint8_t *data, uint32_t length)
{
  uart0.tx(data, length);
}
/*---------------------------------------------------------------------------*/
static Uart*
hw_api_get_uart0(void)
{
  static Uart uart0;
	uart0.open = hw_api_uart0_open;
	uart0.close = hw_api_uart0_close;
	uart0.send = hw_api_uart0_send;
  return &uart0;
}
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
  // PRINTF("[hw arch pin] setup %d value %d\n", pin, value);
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
	// PRINTF("[hw arch pin] output %d value %d\n", pin, value);
  CHECK_PIN_ENABLED(pin);
  switch (value) {
    case PIN_OUTPUT_LOW:
      nrf_gpio_pin_clear(pin);
			// PRINTF("[hw arch pin] output clear %d \n", pin);
    break;
    case PIN_OUTPUT_HIGH:
      nrf_gpio_pin_set(pin);
			// PRINTF("[hw arch pin] output set %d \n", pin);
    break;
		case PIN_OUTPUT_TOGGLE:
			nrf_gpio_pin_toggle(pin);
			// PRINTF("[hw arch pin] output toggle %d \n", pin);
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
hw_pin_attach_interrupt(uint32_t pin, uint32_t edge, PinEventHandler handler)
{
  uint32_t pins_mask = gpioteh.pins_mask;
  uint32_t rising_edge = gpioteh.pins_low_to_high_mask;
  uint32_t falling_edge = gpioteh.pins_high_to_low_mask;

  CHECK_PIN_ENABLED(pin);
  pins_mask = pins_mask | (gpioteh.pins_mask | (1U << pin));

  if (edge & PIN_EDGE_RISING) {
    rising_edge = rising_edge | (gpioteh.pins_mask | (1U << pin));
  }

  if (edge & PIN_EDGE_FALLING) {
    falling_edge = falling_edge | (gpioteh.pins_mask | (1U << pin));
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
Uart*
bsp_hw_uart_api_retrieve(uint8_t id)
{
	if(id == 0) {
		return hw_api_get_uart0();
	}
	return NULL;
}
/*---------------------------------------------------------------------------*/
I2c*
bsp_hw_i2c_api_retrieve(uint8_t id)
{
	return NULL;
}
/*---------------------------------------------------------------------------*/
Spi*
bsp_hw_spi_api_retrieve(uint8_t id)
{
	return NULL;
}
/*---------------------------------------------------------------------------*/
void
hw_uart_terminating(void)
{

}
/*---------------------------------------------------------------------------*/
void
hw_data_source(uint8_t data)
{
  PRINTF("[hw uart api] data: 0x%02x\n", data);
  app_irq_hw_uart_event_t event;
  if (m_uart_handler != NULL) {

    app_irq_event_t irq_event;
    irq_event.event_type = APP_HW_UART_EVENT;
    irq_event.params.hw_uart_event.data = data;
    irq_event.event_hook = hw_uart_rx_irq_hooker;

    // push the timer callback event to user interrupt routin task
    xQueueSend( g_app_irq_queue_handle,  &irq_event, ( TickType_t ) 0 );
    // xQueueSendFromISR( g_app_irq_queue_handle,  &irq_event, ( TickType_t ) 0 );
  }
}
/*---------------------------------------------------------------------------*/
