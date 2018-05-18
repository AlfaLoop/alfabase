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
#include "gpiote.h"
#include "nrf_gpio.h"
#include "sys/clock.h"
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
static bool m_mpu9250_active = false;
static bool m_footpressure_active = false;
typedef struct {
	float value[3];
	int32_t data[3];
	uint32_t timestamp;
} accel_data_t;

/*---------------------------------------------------------------------------*/
void
mpu9250_dmp_data_update(uint32_t source)
{

}
/*---------------------------------------------------------------------------*/
static int
hw_bsp_mpu9250_open(void *args)
{
  if (m_mpu9250_active) {
    return EINVALSTATE;
  }

  SENSOR_MOTIONFUSION.poweron();
  m_mpu9250_active = true;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
hw_bsp_mpu9250_write(const void *buf, uint32_t len, uint32_t *offset)
{

}
/*---------------------------------------------------------------------------*/
static int
hw_bsp_mpu9250_read(void *buf, uint32_t len, uint32_t offset)
{
  int ret;
  static accel_data_t acc;
  uint8_t *user_data = (uint8_t*)buf;
  // uint8_t type = ((uint8_t*)(buf))[0];
  if (offset == 0) {
    SENSOR_MOTIONFUSION.get_accel(acc.value, acc.data, acc.timestamp);
    memcpy(user_data, &acc, len);
  }

  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
hw_bsp_mpu9250_subscribe(void *buf, uint32_t len, HWCallbackHandler handler)
{

}
/*---------------------------------------------------------------------------*/
static int
hw_bsp_mpu9250_close(void *args)
{
  if (!m_mpu9250_active) {
    return EINVALSTATE;
  }
  SENSOR_MOTIONFUSION.poweroff(false);
  m_mpu9250_active = false;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
const static HWDriver hw_drivers[] = {
  {
    /* mpu9250 dmp */
    .name = "mpu9250_dmp",
    .open = hw_bsp_mpu9250_open,
    .write = hw_bsp_mpu9250_write,
    .read = hw_bsp_mpu9250_read,
    .subscribe = hw_bsp_mpu9250_subscribe,
    .close = hw_bsp_mpu9250_close,
  },
  {
    /* iee foot pressure4 */
    .name = "iee_footpressure4",
    .open = hw_bsp_iee_footpressure4_open,
    .write = hw_bsp_iee_footpressure4_write,
    .read = hw_bsp_iee_footpressure4_read,
    .subscribe = hw_bsp_iee_footpressure4_subscribe,
    .close = hw_bsp_iee_footpressure4_close,
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
