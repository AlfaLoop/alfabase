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
#include "frameworks/hw/hw_uart_api.h"
#include <stdint.h>
#include <stdbool.h>
#include "frameworks/app_eventpool.h"
#include "frameworks/app_lifecycle.h"
#include "nrf_gpio.h"
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
extern const struct uart_driver uart0;
static UartRxHandler m_uart_handler = NULL;
/*---------------------------------------------------------------------------*/
static void
hw_uart_rx_irq_hooker(void *ptr)
{
	HwUartEvent *p_event = (HwUartEvent *)ptr;
	if (m_uart_handler != NULL) {
		m_uart_handler(p_event->data);
	}
}
/*---------------------------------------------------------------------------*/
static void
uart_rx_handler(uint8_t data)
{
  PRINTF("[hw-arch] rx data: %02x\n", data);
	if (m_uart_handler != NULL) {

		app_irq_event_t irq_event;
		irq_event.event_type = APP_HW_UART_EVENT;
    irq_event.params.hwUartEvent.data = data;
		irq_event.event_hook = hw_uart_rx_irq_hooker;

		// push the timer callback event to user interrupt routin task
		//xQueueSend( g_app_irq_queue_handle,  &irq_event, ( TickType_t ) 0 );
    xQueueSendFromISR( g_app_irq_queue_handle,  &irq_event, ( TickType_t ) 0 );
	}
}
/*---------------------------------------------------------------------------*/
static uart_config_t m_uart0_arch_config = {
	.tx = TX_PIN_NUMBER,
	.rx = RX_PIN_NUMBER,
	.cts = CTS_PIN_NUMBER,
	.rts = RTS_PIN_NUMBER,
	.baudrate = UART_BAUDRATE_BAUDRATE_Baud9600,
	.hwfc = false,
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

  PRINTF("[hw uart0 arch] UART: %d %d %d\n", pin_tx, pin_rx, nrf_baudrate);

  if (handler != NULL)
    m_uart_handler = handler;

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
hw_api_uart0_send(void)
{

}
/*---------------------------------------------------------------------------*/
Uart*
hw_api_get_uart0(void)
{
  static Uart instance;
	instance.open = hw_api_uart0_open;
	instance.close = hw_api_uart0_close;
	instance.send = hw_api_uart0_send;
  return &instance;
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
void
hw_uart_terminating(void)
{

}
/*---------------------------------------------------------------------------*/
void
hw_data_source(uint8_t data)
{
  PRINTF("[hw uart api] data: 0x%02x\n", data);
  HwUartEvent event;
  if (m_uart_handler != NULL) {

    app_irq_event_t irq_event;
    irq_event.event_type = APP_HW_UART_EVENT;
    irq_event.params.hwUartEvent.data = data;
    irq_event.callback = hw_uart_rx_irq_hooker;

    // push the timer callback event to user interrupt routin task
    xQueueSend( g_app_irq_queue_handle,  &irq_event, ( TickType_t ) 0 );
    // xQueueSendFromISR( g_app_irq_queue_handle,  &irq_event, ( TickType_t ) 0 );
  }
}
/*---------------------------------------------------------------------------*/
