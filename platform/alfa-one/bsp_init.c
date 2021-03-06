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
#include <stdint.h>
#include <stdbool.h>
#include "frameworks/hw/hw_api.h"
#include "frameworks/hw/hw_api_null.h"
#include "frameworks/app_eventpool.h"
#include "frameworks/app_lifecycle.h"
#include "mpu9250-dmp-arch.h"
#include "bsp_ieefp4.h"
#include "bsp_mpudmp.h"
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
const static HWDriver hw_drivers[] = {
  {
    /* mpu9250 dmp */
    .name = "mpu9250_dmp",
    .open = bsp_mpu9250_dmp_open,
    .write = bsp_mpu9250_dmp_write,
    .read = bsp_mpu9250_dmp_read,
    .subscribe = bsp_mpu9250_dmp_subscribe,
    .close = bsp_mpu9250_dmp_close,
  },
  {
    /* iee foot pressure4 */
    .name = "ieefp4",
    .open = bsp_ieefp4_open,
    .write = bsp_ieefp4_write,
    .read = bsp_ieefp4_read,
    .subscribe = bsp_ieefp4_subscribe,
    .close = bsp_ieefp4_close,
  },
};
/*---------------------------------------------------------------------------*/
int
hw_api_bsp_num(void)
{
  return 2;
}
/*---------------------------------------------------------------------------*/
HWDriver*
hw_api_bsp_get(uint32_t idx)
{
  return &hw_drivers[idx];
}
/*---------------------------------------------------------------------------*/
HWDriver*
hw_api_bsp_pipe(const char *dev)
{
  for (int i = 0; i < hw_api_bsp_num(); i++) {
    if (!strcmp(hw_drivers[i].name, dev)) {
      return &hw_drivers[i];
    }
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
