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
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_uart.h"
#include "SEGGER_RTT.h"
#include "dev/logger.h"
#include "dev/uart.h"
#include "bsp_init.h"
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
#if defined(USE_UART0)
extern const struct uart_driver uart0;
#endif
/*---------------------------------------------------------------------------*/
#if defined(USE_LOGGER)
const uart_config_t logger_uart_config = {
	.tx = LOGGER_UART_TX,
	.rx = LOGGER_UART_RX,
	.cts = 0u,
	.rts = 0u,
	.baudrate = UART_BAUDRATE_BAUDRATE_Baud115200,
	.hwfc = false,
	.cb = NULL
};
#endif
/*---------------------------------------------------------------------------*/
void
logger_rtt_printf(char *p_buf, int n)
{
#if defined(DEBUG_ENABLE)
  SEGGER_RTT_WriteString(0, p_buf);
#endif  /* DEBUG_ENABLE */
}
/*---------------------------------------------------------------------------*/
void
logger_serial_enable(void)
{
#if defined(USE_UART0)
  uart0.init(&logger_uart_config);
#endif
}
/*---------------------------------------------------------------------------*/
void
logger_serial_printf(char *p_buf, int n)
{
#if defined(USE_UART0)
  // uart0.tx_with_config(&logger_uart_config, p_buf, n);
  uart0.tx(p_buf, n);
#endif
}
/*---------------------------------------------------------------------------*/
void
logger_serial_disable(void)
{
#if defined(USE_UART0)
  uart0.disable();
#endif
}
/*---------------------------------------------------------------------------*/
