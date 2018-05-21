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
#include "bsp_mpudmp.h"
#include <stdint.h>
#include <stdbool.h>
#include "frameworks/hw/hw_api.h"
#include "frameworks/hw/hw_api_null.h"
#include "frameworks/app_eventpool.h"
#include "frameworks/app_lifecycle.h"
#include "mpu9250-dmp-arch.h"
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
mpu9250_dmp_data_update(uint8_t source)
{
  // PRINTF("[bsp mpudmp] data update %d\n", source);
  static motion_data_event_t event;

  if (m_mpu9250_active) {
    if (m_sensor_event_callback != NULL) {
      event.type = source;

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
bsp_mpu9250_dmp_open(void *args)
{
  if (m_mpu9250_active) {
    return EINVALSTATE;
  }

  SENSOR_MOTIONFUSION.poweron();
  m_mpu9250_active = true;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
bsp_mpu9250_dmp_write(const void *buf, uint32_t len, uint32_t offset)
{
  uint8_t *p_attr = (uint8_t*)buf;
  if (offset == DEV_MOTION_CONFIG_SAMPLEING_RATE_TYPE) {
    SENSOR_MOTIONFUSION.config_update(DEV_MOTION_CONFIG_SAMPLEING_RATE_TYPE, p_attr[0]);
  } else if (offset == DEV_MOTION_CONFIG_RESET_PEDOMETER_TYPE) {
    SENSOR_MOTIONFUSION.config_update(DEV_MOTION_CONFIG_RESET_PEDOMETER_TYPE, 0);
  }
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
bsp_mpu9250_dmp_read(void *buf, uint32_t len, uint32_t offset)
{
  switch (offset) {
    case 0:
    {
      // The acceleration force in ‘m/s’ that is applied to a device on all three physical axes (x, y, and z), including the force of gravity.
      mems_data_t *p_acc_data = (mems_data_t *)buf;
      SENSOR_MOTIONFUSION.get_accel(p_acc_data->value, p_acc_data->data, &p_acc_data->timestamp);
    }
    break;
    case 1:
    {
      mems_data_t *p_gyro_data = (mems_data_t *)buf;
      SENSOR_MOTIONFUSION.get_gyro(p_gyro_data->value, p_gyro_data->data, &p_gyro_data->timestamp);
    }
    break;
    case 2:
    {
      mems_data_t *p_compass_data = (mems_data_t *)buf;
      SENSOR_MOTIONFUSION.get_compass(p_compass_data->value, p_compass_data->data, &p_compass_data->timestamp);
    }
    break;
    case 3:
    {
      quat_data_t *p_quat_data = (quat_data_t *)buf;
      SENSOR_MOTIONFUSION.get_quaternion(p_quat_data->value, p_quat_data->data, &p_quat_data->timestamp);
    }
    break;
    case 4:
    {
      mems_data_t *p_euler_data = (mems_data_t *)buf;
      SENSOR_MOTIONFUSION.get_euler(p_euler_data->value, p_euler_data->data, &p_euler_data->timestamp);
    }
    break;
    case 5:
    {
      // The acceleration force in ‘m/s’ that is applied to a device on all three physical axes (x, y, and z), excluding the force of gravity.
      linear_accel_data_t *p_linear_accel_data = (linear_accel_data_t *)buf;
      SENSOR_MOTIONFUSION.get_linear_accel(p_linear_accel_data->value, &p_linear_accel_data->timestamp);
    }
    break;
    case 6:
    {
      gravity_vector_t *p_gravity_vector_data = (gravity_vector_t *)buf;
      SENSOR_MOTIONFUSION.get_gravity_vector(p_gravity_vector_data->value, &p_gravity_vector_data->timestamp);
    }
    break;
    case 7:
    {
      heading_data_t *p_heading_data = (heading_data_t *)buf;
      SENSOR_MOTIONFUSION.get_heading(&p_heading_data->value, &p_heading_data->data, &p_heading_data->timestamp);
    }
    break;
  }
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
bsp_mpu9250_dmp_subscribe(void *buf, uint32_t len, HWCallbackHandler handler)
{
  m_sensor_event_callback = handler;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
bsp_mpu9250_dmp_close(void *args)
{
  if (!m_mpu9250_active) {
    return EINVALSTATE;
  }
  SENSOR_MOTIONFUSION.poweroff(false);
  m_mpu9250_active = false;
  m_sensor_event_callback = NULL;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static void
app_terminating(void)
{
	bsp_mpu9250_dmp_close(NULL);
}
/*---------------------------------------------------------------------------*/
static struct app_lifecycle_event lifecycle_event = {
	.name = "hw_bsp_mpudmp",
	.terminating = app_terminating
};
/*---------------------------------------------------------------------------*/
int
bsp_mpu9250_dmp_init(void)
{
	app_lifecycle_register(&lifecycle_event);

  mpu9250_dmp_config_t mpu9250_dmp_config = {
		.data_source = mpu9250_dmp_data_update
	};
	SENSOR_MOTIONFUSION.init(&mpu9250_dmp_config);
	SENSOR_MOTIONFUSION.poweroff(false);
  // SENSOR_MOTIONFUSION.poweron();
}
/*---------------------------------------------------------------------------*/
