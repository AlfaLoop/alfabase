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
#include "hw-arch.h"
#include <stdint.h>
#include <stdbool.h>
#include "frameworks/app_eventpool.h"
#include "frameworks/app_lifecycle.h"
#include "gpiote.h"
#include "nrf_gpio.h"
#include "sys/clock.h"
#include "sys/ctimer.h"
#include "sys/devid.h"
#include "dev/uart.h"
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
#define CHECK_PIN_ENABLED(pin)                                    \
   do                                                             \
   {                                                              \
      if (!hw_arch_is_pin_enabled(pin))                              \
          return ENOSUPPORT;                                      \
   } while (0)

/*---------------------------------------------------------------------------*/
extern const struct uart_driver uart0;
static UartRxHandler m_uart_handler = NULL;
static PinWatchtHandler m_pin_watch_handler = NULL;
static uint8_t *m_enable_pin_list;
static uint32_t m_enable_pin_size;
static bool m_interrupt_enable = false;
/*---------------------------------------------------------------------------*/
static void
hw_uart_rx_irq(void *ptr)
{
	UartEvent *p_event = (UartEvent *)ptr;
	if (m_uart_handler != NULL) {
		m_uart_handler(p_event->data);
	}
}
/*---------------------------------------------------------------------------*/
static void
uart_rx_handler(uint8_t data)
{
  PRINTF("[hw-arch] rx data: %02x\n", data);
  UartEvent event;
	if (m_uart_handler != NULL) {

		AppIrqEvent appIrqEvent;
		appIrqEvent.event_type = APP_HW_UART_EVENT;
    appIrqEvent.params.uartEvent.data = data;
		appIrqEvent.callback = hw_uart_rx_irq;

		// push the timer callback event to user interrupt routin task
		//xQueueSend( xAppIrqQueueHandle,  &appIrqEvent, ( TickType_t ) 0 );
    xQueueSendFromISR( xAppIrqQueueHandle,  &appIrqEvent, ( TickType_t ) 0 );
	}
}
/*---------------------------------------------------------------------------*/
static uart_config_t m_uart_arch_config = {
	.tx = TX_PIN_NUMBER,
	.rx = RX_PIN_NUMBER,
	.cts = CTS_PIN_NUMBER,
	.rts = RTS_PIN_NUMBER,
	.baudrate = UART_BAUDRATE_BAUDRATE_Baud9600,
	.hwfc = false,
	.cb = uart_rx_handler
};
/*---------------------------------------------------------------------------*/
static void
hw_pin_irq(void *ptr)
{
	PinWatchEvent *p_event = (PinWatchEvent *)ptr;
	if (m_pin_watch_handler != NULL) {
		m_pin_watch_handler(p_event->pin_no);
	}
}
/*---------------------------------------------------------------------------*/
static void
gpio_int_event_handler(uint32_t pin_no)
{
	PinWatchEvent event;
	if (m_pin_watch_handler != NULL) {

		AppIrqEvent appIrqEvent;
		appIrqEvent.event_type = APP_HW_PIN_EVENT;
    appIrqEvent.params.pinWatchEvent.pin_no = pin_no;
		appIrqEvent.callback = hw_pin_irq;

		// push the timer callback event to user interrupt routin task
		xQueueSend( xAppIrqQueueHandle,  &appIrqEvent, ( TickType_t ) 0 );
	}
}
/*---------------------------------------------------------------------------*/
static gpiote_handle_t gpioteh = {.event_handler=gpio_int_event_handler,
								  .pins_mask = 0,
								  .pins_low_to_high_mask= 0,
								  .pins_high_to_low_mask= 0,
								  .sense_high_pins = 0 };
/*---------------------------------------------------------------------------*/
static bool
hw_arch_is_pin_enabled(uint32_t pin)
{
  bool enabled = false;
  for (int i = 0; i < m_enable_pin_size; i++) {
    // PRINTF("[hw arch] check pin %d\n", m_enable_pin_list[i]);
    if (m_enable_pin_list[i] == pin) {
      enabled = true;
      break;
    }
  }
  return enabled;
}
/*---------------------------------------------------------------------------*/
int
hw_arch_pinSet(uint32_t pin, uint8_t value)
{
  // PRINTF("[hw arch] pinSet %d value %d\n", pin, value);
  CHECK_PIN_ENABLED(pin);

  switch (value) {
    case PIN_INPUT_NOPULL:
      nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_NOPULL);
    break;
    case PIN_INPUT_PULLUP:
      nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_PULLUP);
    break;
    case PIN_INPUT_PULLDOWN:
      nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_PULLDOWN);
    break;
    case PIN_OUTPUT_LOW:
      nrf_gpio_cfg_output(pin);
      nrf_gpio_pin_clear(pin);
    break;
    case PIN_OUTPUT_HIGH:
      nrf_gpio_cfg_output(pin);
      nrf_gpio_pin_set(pin);
    break;
    default:
      return EINVAL;
    break;
  }
	return ENONE;
}
/*---------------------------------------------------------------------------*/
int
hw_arch_pinRead(uint32_t pin, uint8_t *value)
{
  CHECK_PIN_ENABLED(pin);
  *value = nrf_gpio_pin_read(pin);
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
hw_arch_pinInfo(uint8_t *pin, uint8_t *size)
{
  pin = m_enable_pin_list;
  *size = m_enable_pin_size;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
hw_arch_pinWatchSet(uint32_t pin_mask, uint32_t edge_mask, PinWatchtHandler handler)
{
  bool enable = false;
  uint32_t pin_no;
  uint32_t edge;
  gpioteh.pins_mask = 0;

  for (pin_no = 0; pin_no < NO_OF_PINS; pin_no++){
    uint32_t mask = (1 << pin_no);
    if ((mask & pin_mask) != 0){
      CHECK_PIN_ENABLED(pin_no);
      gpioteh.pins_mask = (gpioteh.pins_mask | (1U << pin_no));

      edge = (edge_mask & mask);
      if (edge == PIN_EDGE_RISING) {
        gpioteh.pins_low_to_high_mask = (gpioteh.pins_mask | (1U << pin_no));
      } else if (edge == PIN_EDGE_FALLING) {
        gpioteh.pins_high_to_low_mask = (gpioteh.pins_mask | (1U << pin_no));
      }
      enable = true;
    }
  }

  if (enable) {
    gpiote_register(&gpioteh);
    m_interrupt_enable = true;
  }
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
hw_arch_pinWatchClose(void)
{
  gpiote_unregister(&gpioteh);
  m_interrupt_enable = false;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
hw_arch_uartInit(uint32_t pin_tx, uint32_t pin_rx, uint32_t baudrate, UartRxHandler handler)
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

  m_uart_arch_config.tx = pin_tx;
  m_uart_arch_config.rx = pin_rx;
  m_uart_arch_config.baudrate = nrf_baudrate;

  PRINTF("[hw-arch] UART: %d %d %d\n", pin_tx, pin_rx, nrf_baudrate);

  if (handler != NULL)
    m_uart_handler = handler;

  uart0.init(&m_uart_arch_config);
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
hw_arch_uartDisable(void)
{
  m_uart_handler = NULL;
  uart0.disable();
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
hw_arch_uartSend(uint8_t *data, uint32_t length)
{
  PRINTF("[hw-arch] uart send %d\n", length);
	for (uint32_t i = 0; i < length; i++) {
		PRINTF("%2x ", data[i]);
	}
	PRINTF("\n");
	uart0.tx(data, length);
	return ENONE;
}
/*---------------------------------------------------------------------------*/
static void
app_terminating(void)
{
  if (m_uart_handler != NULL)
    m_uart_handler = NULL;
  if (m_pin_watch_handler != NULL)
    m_pin_watch_handler = NULL;

  if (m_interrupt_enable) {
    gpiote_unregister(&gpioteh);
    m_interrupt_enable = false;
  }

  for (int i = 0; i < m_enable_pin_size; i++) {
    nrf_gpio_cfg_default(m_enable_pin_list[i]);
  }
}
/*---------------------------------------------------------------------------*/
static struct app_lifecycle_event lifecycle_event = {
	.name = "hw_arch",
	.terminating = app_terminating
};
/*---------------------------------------------------------------------------*/
int
hw_api_arch_init(const uint8_t *enabled_pin_list, uint32_t size)
{
	app_lifecycle_register(&lifecycle_event);

  m_enable_pin_list = enabled_pin_list;
  m_enable_pin_size = size;
  PRINTF("[hw-arch] enable pin size: %d\n", size);
}
/*---------------------------------------------------------------------------*/
