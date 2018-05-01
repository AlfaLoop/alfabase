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
#include "oslogger_api.h"
#include "frameworks/app_lifecycle.h"
#include "frameworks/app_eventpool.h"
#include "loader/symtab.h"
#include "dev/syslog.h"
#include "dev/rtt.h"
#include "dev/logger.h"
#include "errno.h"

#include "FreeRTOS.h"
#include "task.h"
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
static bool uart_enable = false;

/*---------------------------------------------------------------------------*/
int
oslog_printf(uint32_t interface, const char *fmt, ...)
{
	char *p;
	char buffer[256];
	va_list args;
	int offset=0, n=0, k=0;

	va_start(args, fmt);
	n = ee_vsprintf(buffer, fmt, args);
	va_end(args);

	if ((interface & LOG_RTT)) {
		logger_rtt_printf(buffer, n);
	}

	if ((interface & LOG_SERIAL)) {
		if (!uart_enable) {
			uart_enable = true;
			logger_serial_enable();
		}
		logger_serial_printf(buffer, n);
	}
	return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
oslog_sprintf(char *buf, const char *fmt, ...)
{
	va_list args;
	int n = 0;
	va_start(args, fmt);
	n = ee_vsprintf(buf, fmt, args);
	va_end(args);
	return n;
}
/*---------------------------------------------------------------------------*/
Logger*
OSLogger(void)
{
	static Logger log;
	log.printf = oslog_printf;
	log.sprintf = oslog_sprintf;
	return &log;
}
static struct symbols symbolOSLogger = {
	.name = "OSLogger",
	.value = (void *)&OSLogger
};
/*---------------------------------------------------------------------------*/
static void
app_terminating(void)
{
	if (uart_enable) {
		logger_serial_disable();
		uart_enable = false;
	}
}
/*---------------------------------------------------------------------------*/
static struct app_lifecycle_event lifecycle_event = {
	.name = "oslogger_api",
	.terminating = app_terminating
};
/*---------------------------------------------------------------------------*/
void
oslogger_api_init(void)
{
	app_lifecycle_register(&lifecycle_event);
	symtab_add(&symbolOSLogger);
}
/*---------------------------------------------------------------------------*/
