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
#include "oslocaltime_api.h"
#include "frameworks/app_lifecycle.h"
#include "frameworks/app_eventpool.h"
#include "loader/symtab.h"
#include "sys/systime.h"
#include "errno.h"
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
static uint32_t
osclock_get_seconds(void)
{
  return systime_seconds();
}
/*---------------------------------------------------------------------------*/
static uint32_t
osclock_get_minutes(void)
{
  return systime_minutes();
}
/*---------------------------------------------------------------------------*/
static uint32_t
osclock_get_hour24(void)
{
  return systime_hour_24();
}
/*---------------------------------------------------------------------------*/
static uint32_t
osclock_get_hour12(void)
{
  return systime_hour_12();
}
/*---------------------------------------------------------------------------*/
static uint32_t
osclock_get_day_of_week(void)
{
  return systime_day_of_week();
}
/*---------------------------------------------------------------------------*/
static uint32_t
osclock_get_day_of_month(void)
{
  return systime_day_of_month();
}
/*---------------------------------------------------------------------------*/
static uint32_t
osclock_get_day_of_year(void)
{
  return systime_day_of_year();
}
/*---------------------------------------------------------------------------*/
static uint32_t
osclock_get_month(void)
{
  return systime_month();
}
/*---------------------------------------------------------------------------*/
static uint32_t
osclock_get_year(void)
{
  return systime_year();
}
/*---------------------------------------------------------------------------*/
Localtime*
OSLocaltime(void)
{
	static Localtime time;
  time.getSeconds = osclock_get_seconds;
  time.getMinutes = osclock_get_minutes;
  time.getHour24 = osclock_get_hour24;
  time.getHour12 = osclock_get_hour12;
  time.getDayOfWeek = osclock_get_day_of_week;
  time.getDayOfMonth = osclock_get_day_of_month;
  time.getDayOfYear = osclock_get_day_of_year;
  time.getMonth = osclock_get_month;
  time.getYear = osclock_get_year;
	return &time;
}
static struct symbols symbolOSLocaltime = {
	.name = "OSLocaltime",
	.value = (void *)&OSLocaltime
};
/*---------------------------------------------------------------------------*/
void
oslocaltime_api_init(void)
{
	symtab_add(&symbolOSLocaltime);
}
/*---------------------------------------------------------------------------*/
