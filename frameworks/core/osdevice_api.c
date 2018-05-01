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
#include "osdevice_api.h"
#include "frameworks/app_lifecycle.h"
#include "frameworks/app_eventpool.h"
#include "loader/symtab.h"
#include "contiki-version.h"
#include "sys/devid.h"
#include "sys/pm.h"
#include "bsp_init.h"
#include "errno.h"
#include "FreeRTOS.h"
#include "task.h"
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
int
osdevice_gethwid(uint8_t *hwid, int max_len)
{
  if (max_len > DEVICE_HWID_MAX_LEN)
    max_len = DEVICE_HWID_MAX_LEN;
  hwid_gen(hwid, max_len);

  return ENONE;
}
/*---------------------------------------------------------------------------*/
char*
osdevice_get_platform_name(void)
{
  return PLATFORM_DEVICE_NAME;
}
/*---------------------------------------------------------------------------*/
uint8_t
osdevice_get_power_status(void)
{
  return pm_get_battery();
}
/*---------------------------------------------------------------------------*/
char*
osdevice_get_kernel_version(void)
{
  return ALFABASE_VERSION_STRING;
}
/*---------------------------------------------------------------------------*/
Device*
OSDevice(void)
{
	static Device d;
	d.getHwid = osdevice_gethwid;
	d.getPlatformName = osdevice_get_platform_name;
  d.getKernelVersion = osdevice_get_kernel_version;
  d.getBatteryLevel = osdevice_get_power_status;
	return &d;
}
static struct symbols symbolOSDevice = {
	.name = "OSDevice",
	.value = (void *)&OSDevice
};
/*---------------------------------------------------------------------------*/
void
osdevice_api_init(void)
{
	symtab_add(&symbolOSDevice);
}
/*---------------------------------------------------------------------------*/
