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
#include "sys/systime.h"
#include "spiffs/spiffs.h"
#include "contiki.h"
#include "errno.h"
#include "dev/watchdog.h"
#include "libs/util/linklist.h"
#include <time.h>
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
LIST(period_list);
/*---------------------------------------------------------------------------*/
static struct ctimer m_systime_system_timer;
static uint32_t	m_current_time;
static uint16_t m_adjust_time = 0;
/*
http://www.cplusplus.com/reference/ctime/mktime/
https://bryceknowhow.blogspot.tw/2013/10/cc-gmtimemktimetimetstruct-tm.html
*/
/*---------------------------------------------------------------------------*/
static void
period_event_notify(void)
{
	systime_period_event_t *s;
	for(s = list_head(period_list);
		s != NULL;
		s = list_item_next(s))
	{
		s->callback();
	}
}
/*---------------------------------------------------------------------------*/
// static void
// timer_handler(void *in)
// {
// 	// ctimer_set(&m_systime_system_timer, 1000, timer_handler, (void *)NULL);
// 	watchdog_periodic();
// 	// ctimer_reset(&m_systime_system_timer);
//   m_current_time++;
//   PRINTF("[systime] time %d clock_time %d\n", m_current_time, clock_time());
// 	period_event_notify();
// }
/*---------------------------------------------------------------------------*/
uint32_t
systime_in_seconds(void)
{
  struct tm* time_struct = localtime(&m_current_time);
  return time_struct->tm_hour*3600 + time_struct->tm_min*60 + time_struct->tm_sec;
}
/*---------------------------------------------------------------------------*/
uint32_t
systime_hour_24(void)
{
  struct tm* time_struct = localtime(&m_current_time);
  return time_struct->tm_hour;
}
/*---------------------------------------------------------------------------*/
uint32_t
systime_hour_12(void)
{
  struct tm* time_struct = localtime(&m_current_time);
 	  uint32_t h = time_struct->tm_hour % 12;
 		return h == 0 ? 12 : h;
}
/*---------------------------------------------------------------------------*/
uint32_t
systime_hour_12_designator(void)
{
  struct tm* time_struct = localtime(&m_current_time);
 	  return time_struct->tm_hour / 12;
}
/*---------------------------------------------------------------------------*/
uint32_t
systime_minutes(void)
{
  struct tm* time_struct = localtime(&m_current_time);
 	  return time_struct->tm_min;
}
/*---------------------------------------------------------------------------*/
uint32_t
systime_seconds(void)
{
  struct tm* time_struct = localtime(&m_current_time);
  return time_struct->tm_sec;
}
/*---------------------------------------------------------------------------*/
uint32_t
systime_day_of_week(void)
{
  struct tm* time_struct = localtime(&m_current_time);
  return time_struct->tm_wday + 1;
}
/*---------------------------------------------------------------------------*/
uint32_t
systime_day_of_month(void)
{
  struct tm* time_struct = localtime(&m_current_time);
  return time_struct->tm_mday;
}
/*---------------------------------------------------------------------------*/
uint32_t
systime_day_of_year(void)
{
  struct tm* time_struct = localtime(&m_current_time);
  return time_struct->tm_yday + 1;
}
/*---------------------------------------------------------------------------*/
uint32_t
systime_month(void)
{
  struct tm* time_struct = localtime(&m_current_time);
  return time_struct->tm_mon + 1;
}
/*---------------------------------------------------------------------------*/
uint32_t
systime_year(void)
{
  struct tm* time_struct = localtime(&m_current_time);
  return time_struct->tm_year + 1900;
}
/*---------------------------------------------------------------------------*/
uint32_t
systime_epoch_time(void)
{
	return m_current_time;
}
/*---------------------------------------------------------------------------*/
int
systime_period_event_register(systime_period_event_t *event)
{
	if (event == NULL)
		return EINVAL;

	list_add(period_list, event);
	return ENONE;
}
/*---------------------------------------------------------------------------*/
int
systime_set_current_time(uint32_t t)
{
	spiffs_file fd;
	spiffs_stat stat;
	int ret;
  // ctimer_stop(&m_systime_system_timer);
	m_current_time = t;
	fd = SPIFFS_open(&SYSFS, ".systime", SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_WRONLY, 0);
	if(fd < 0) {
		SPIFFS_close(&SYSFS, fd);
		// setup a callback function to check filesystem
	  // ctimer_set(&m_systime_system_timer, CLOCK_SECOND  , timer_handler, (void *)NULL);
		return EINTERNAL;
	} else {
			ret = SPIFFS_write(&SYSFS, fd, (u8_t *)&m_current_time, 4);
			if (ret < 0 ) {
				PRINTF("[systime] write .systime error %i\n", SPIFFS_errno(&SYSFS));
				SPIFFS_close(&SYSFS, fd);
				// setup a callback function to check filesystem
			  // ctimer_set(&m_systime_system_timer, CLOCK_SECOND  , timer_handler, (void *)NULL);
				return EINTERNAL;
			}
	}
	SPIFFS_close(&SYSFS, fd);

	// setup a callback function to check filesystem
  // ctimer_set(&m_systime_system_timer, CLOCK_SECOND  , timer_handler, (void *)NULL);
	return ENONE;
}
/*---------------------------------------------------------------------------*/
void
systime_update_current_time(uint32_t year, uint32_t month, uint32_t day, uint32_t hour, uint32_t minute)
{
	/*
	tm_sec	int	seconds after the minute	0-60*
	tm_min	int	minutes after the hour	0-59
	tm_hour	int	hours since midnight	0-23
	tm_mday	int	day of the month	1-31
	tm_mon	int	months since January	0-11
	tm_year	int	years since 1900
	tm_wday	int	days since Sunday	0-6
	tm_yday	int	days since January 1	0-365
	tm_isdst	int	Daylight Saving Time flag
	*/
	struct tm timeinfo;
	timeinfo.tm_year = year - 1900;
	timeinfo.tm_mon = month - 1;
	timeinfo.tm_mday = day;
	timeinfo.tm_hour = hour;
	timeinfo.tm_min = minute;
	timeinfo.tm_sec = 0;

	PRINTF("[systime] year %04d, month %2d, day of month %2d hour %2d minute %2d\n",
		year, month, day, hour, minute);
	m_current_time = mktime ( &timeinfo );
	systime_set_current_time(m_current_time);
}
/*---------------------------------------------------------------------------*/
PROCESS(systime_process, "systime_process");
/*---------------------------------------------------------------------------*/
void
systime_init(void)
{
	PRINTF("[systime] systime init\n");
	spiffs_file fd;
	spiffs_stat stat;
	int res = SPIFFS_stat(&SYSFS, ".systime", &stat);
	if (res != SPIFFS_OK) {
		// fd = SPIFFS_open(&SYSFS, ".systime", SPIFFS_CREAT | SPIFFS_RDWR, 0);
		// m_current_time = clock_time();
		// if (SPIFFS_write(&SYSFS, fd, (u8_t *)&m_current_time, 4) < 0) {
		// 	PRINTF("[systime] write boot.py error %i\n", SPIFFS_errno(&SYSFS));
		// }
		systime_update_current_time(2018, 2, 27, 17, 20);
	} else {
		PRINTF("[systime] .systime exist\n");
		fd = SPIFFS_open(&SYSFS, ".systime", SPIFFS_RDWR, 0);
		SPIFFS_read(&SYSFS, fd, (u8_t *)&m_current_time, 4);
		PRINTF("[systime] .systime current time %d\n", m_current_time);
	}
	SPIFFS_close(&SYSFS, fd);

	list_init(period_list);

  // setup a callback function to check filesystem
  // ctimer_set(&m_systime_system_timer, CLOCK_SECOND, timer_handler, (void *)NULL);
	if (!process_is_running(&systime_process))
		process_start(&systime_process, NULL);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(systime_process, ev, data)
{
	static struct etimer st;
	PROCESS_BEGIN();

	etimer_set(&st, CLOCK_SECOND);
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&st));
		watchdog_periodic();
	  m_current_time++;
	  // PRINTF("[systime] time %d clock_time %d\n", m_current_time, clock_time());
		period_event_notify();
		etimer_reset(&st);
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
