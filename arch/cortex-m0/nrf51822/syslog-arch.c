/* Copyright (c) 2016, Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *   * Neither the name of Nordic Semiconductor ASA nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
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
#include "board.h"
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
