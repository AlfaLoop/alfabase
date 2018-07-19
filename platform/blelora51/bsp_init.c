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
#include "bsp_led.h"
#include "bsp_button.h"
#include "bsp_mpuraw.h"
#include "bsp_e32ttl.h"
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
const static HWDriver hw_drivers[] = {
  {
    /* led */
    .name = "led",
    .open = hw_null_open,
    .write = bsp_led_write,
    .read = bsp_led_read,
    .subscribe = hw_null_subscribe,
    .close = hw_null_close,
  },
  {
    /* Button */
    .name = "button",
    .open = hw_null_open,
    .write = hw_null_write,
    .read = bsp_button_read,
    .subscribe = bsp_button_subscribe,
    .close = hw_null_close,
  },
  {
    /* MPU9250 raw data */
    .name = "mpu9250_raw",
    .open = bsp_mpu9250_raw_open,
    .write = bsp_mpu9250_raw_write,
    .read = bsp_mpu9250_raw_read,
    .subscribe = bsp_mpu9250_raw_subscribe,
    .close = bsp_mpu9250_raw_close,
  },
  {
    /* E32-TTL 100 */
    .name = "e32ttl",
    .open = bsp_e32ttl_open,
    .write = bsp_e32ttl_write,
    .read = bsp_e32ttl_read,
    .subscribe = bsp_e32ttl_subscribe,
    .close = bsp_e32ttl_close,
  },
};
/*---------------------------------------------------------------------------*/
int
hw_api_bsp_num(void)
{
  return 4;
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
