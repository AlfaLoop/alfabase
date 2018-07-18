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
#include "bsp_mpuraw.h"
#include <stdint.h>
#include <stdbool.h>
#include "frameworks/hw/hw_api.h"
#include "frameworks/hw/hw_api_null.h"
#include "frameworks/app_eventpool.h"
#include "frameworks/app_lifecycle.h"
#include "mpu9250-raw-arch.h"
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
/*---------------------------------------------------------------------------*/

static bool m_mpu9250_active = false;
static HWCallbackHandler m_sensor_event_callback = NULL;
/*---------------------------------------------------------------------------*/
static void
sensor_api_irq_hooker(void *ptr)
{
  app_irq_hw_event_t *p_event = (app_irq_hw_event_t *) ptr;
	motion_data_event_t *p_data = (motion_data_event_t *)p_event->params;
  // PRINTF("[bsp mpudmp] api irq hooker event %d\n", p_data->type);

	if (m_sensor_event_callback != NULL) {
		m_sensor_event_callback(p_data);
	}
}
/*---------------------------------------------------------------------------*/
void
mpu9250_raw_data_update(int16_t *accel, int16_t *gyro, int16_t *compass, uint32_t timestamp)
{
  // PRINTF("[bsp mpudmp] data update\n");
  static motion_data_event_t event;

  if (m_mpu9250_active) {
    if (m_sensor_event_callback != NULL) {
      event.accel[0] = accel[0];
      event.accel[1] = accel[1];
      event.accel[2] = accel[2];
      event.gyro[0] = gyro[0];
      event.gyro[1] = gyro[1];
      event.gyro[2] = gyro[2];
      event.compass[0] = compass[0];
      event.compass[1] = compass[1];
      event.compass[2] = compass[2];
      event.timestamp = timestamp;

			app_irq_event_t irq_event;
			irq_event.event_type = APP_HW_EVENT;
      irq_event.params.hw_event.params = &event;
			irq_event.event_hook = sensor_api_irq_hooker;
			xQueueSend( g_app_irq_queue_handle,  &irq_event, ( TickType_t ) 0 );
    }
  }
}
/*---------------------------------------------------------------------------*/
int
bsp_mpu9250_raw_open(void *args)
{
  if (m_mpu9250_active) {
    return EINVALSTATE;
  }

  SENSOR_MPU9250.poweron();
  m_mpu9250_active = true;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
bsp_mpu9250_raw_write(const void *buf, uint32_t len, uint32_t offset)
{
  uint8_t *p_attr = (uint8_t*)buf;
  if (offset == DEV_MOTION_CONFIG_SAMPLEING_RATE_TYPE) {
    SENSOR_MPU9250.config_update(DEV_MOTION_CONFIG_SAMPLEING_RATE_TYPE, p_attr[0]);
  }
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
bsp_mpu9250_raw_read(void *buf, uint32_t len, uint32_t offset)
{
  mems_data_t *p_mems_data = (mems_data_t *)buf;
  switch (offset) {
    case 0:
    {
      SENSOR_MPU9250.get_accel(&p_mems_data->value[0], &p_mems_data->timestamp);
    }
    break;
    case 1:
    {
      SENSOR_MPU9250.get_gyro(&p_mems_data->value[0], &p_mems_data->timestamp);
    }
    break;
    case 2:
    {
      SENSOR_MPU9250.get_compass(p_mems_data->value, &p_mems_data->timestamp);
    }
    break;
  }
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
bsp_mpu9250_raw_subscribe(void *buf, uint32_t len, HWCallbackHandler handler)
{
  m_sensor_event_callback = handler;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
bsp_mpu9250_raw_close(void *args)
{
  if (!m_mpu9250_active) {
    return EINVALSTATE;
  }
  SENSOR_MPU9250.poweroff(false);
  m_mpu9250_active = false;
  m_sensor_event_callback = NULL;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static void
app_terminating(void)
{
	bsp_mpu9250_raw_close(NULL);
}
/*---------------------------------------------------------------------------*/
static struct app_lifecycle_event lifecycle_event = {
	.name = "hw_bsp_mpuraw",
	.terminating = app_terminating
};
/*---------------------------------------------------------------------------*/
int
bsp_mpu9250_raw_init(void)
{
	app_lifecycle_register(&lifecycle_event);

  mpu9250_raw_config_t mpu9250_raw_config = {
		.data_source = mpu9250_raw_data_update
	};
	SENSOR_MPU9250.init(&mpu9250_raw_config);
	SENSOR_MPU9250.poweroff(false);
}
/*---------------------------------------------------------------------------*/
