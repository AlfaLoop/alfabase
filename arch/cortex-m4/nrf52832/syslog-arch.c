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
#include "loader/symtab.h"
#include "dev/syslog.h"
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
#ifdef NRF_LOG_USES_RTT

#include "SEGGER_RTT.h"
int
syslog(const char *fmt, ...)
{
	char buf[256],*p;
	va_list args;
	int n=0;

	va_start(args, fmt);
	ee_vsprintf(buf, fmt, args);
	va_end(args);

	p=buf;
	while (*p) {
		n++;
		p++;
	}
	SEGGER_RTT_WriteString(0, buf);
	return n;
}

#else

#include "dev/uart.h"
#include "nrf_delay.h"
#include "nrf_uart.h"
#include "bsp_init.h"
/*---------------------------------------------------------------------------*/
extern const struct uart_driver uart0;
/*---------------------------------------------------------------------------*/
const uart_config_t printf_config = {
	.tx = UART_TX,
	.rx = UART_RX,
	.cts = 0u,
	.rts = 0u,
	.baudrate = UART_BAUDRATE_BAUDRATE_Baud38400,
	.hwfc = false,
	.cb = NULL
};
/*---------------------------------------------------------------------------*/
int
syslog(const char *fmt, ...)
{
	char buf[256],*p;
	va_list args;
	int n=0;
	uart_config_t *config;

	va_start(args, fmt);
	ee_vsprintf(buf, fmt, args);
	va_end(args);

	p=buf;
	while (*p) {
		n++;
		p++;
	}
   	uart0.tx_with_config(&printf_config, buf, n);
	nrf_delay_ms(1);
	return n;
}
/*---------------------------------------------------------------------------*/

#endif  /* NRF_LOG_USES_RTT */
