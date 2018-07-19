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
#include "mpu9250-raw-arch.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "dev/i2c.h"
#include "dev/spi.h"
#include "frameworks/app_eventpool.h"
#include "frameworks/app_lifecycle.h"
#include "sensors/inv_mpu/inv_mpu.h"
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
/*---------------------------------------------------------------------------*/
#if defined(INV_MPU_TWI_DRIVER_CONF)
#define TWI INV_MPU_TWI_DRIVER_CONF
#else
#error Need define INV_MPU_TWI_DRIVER_CONF driver instance
#endif
extern const struct i2c_driver TWI;

/*---------------------------------------------------------------------------*/
#define DEFAULT_MPU_HZ  (20)

#define MPU9250_ACCEL_COUNTS_2G     			0.00006;
#define MPU9250_ACCEL_COUNTS_4G     			0.00012;
#define MPU9250_ACCEL_COUNTS_8G     			0.00024;
#define MPU9250_ACCEL_COUNTS_16G     			0.00048;

#define MPU9250_GYRO_COUNTS_250     			0.00762;
#define MPU9250_GYRO_COUNTS_500     			0.01525;
#define MPU9250_GYRO_COUNTS_1000     			0.03051;
#define MPU9250_GYRO_COUNTS_2000     			0.06103;

#define MPU9250_COMPASS_COUNTS_4800ut     0.58593;

static uint32_t  new_timestamp;
static bool 	 motion_active = false;
static uint8_t   cfg_sample_rate = 20;
static unsigned short  cfg_gyro_fsr = 250;
static unsigned short  cfg_accel_fsr = 2;
static short     inst_accel_data[3];
static short     inst_gyro_data[3];
static short     inst_mag_data[3];
static mpu9250_raw_data_update_func_t	m_framework_raw_data_func;
PROCESS(mpu9250_process, "Mpu9250 process");
process_event_t mpu9250_sensor_event;
/*---------------------------------------------------------------------------*/
static void
mpu9250_int_event_handler(gpiote_event_t *event)
{
  uint32_t pin_mask = (1u << MPU_INT);
  if (event->pin_no == pin_mask) {
    if (motion_active) {
			mpu_get_gyro_reg(inst_gyro_data, NULL);
			mpu_get_accel_reg(inst_accel_data, NULL);
			mpu_get_compass_reg(inst_mag_data, NULL);
			new_timestamp = clock_time();
      // PRINTF("[mpu9250 raw] new timestamp %d\n", new_timestamp);
			process_post(&mpu9250_process, mpu9250_sensor_event, NULL);
    }
  }
}
/*---------------------------------------------------------------------------*/
static gpiote_handle_t gpioteh = {.event_handler=mpu9250_int_event_handler,
								  .pins_mask = (1U << MPU_INT),
								  .pins_low_to_high_mask= (1U << MPU_INT),
								  .pins_high_to_low_mask= 0,
								  .sense_high_pins = 0 };
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mpu9250_process, ev, data)
{
	PROCESS_BEGIN();
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(ev == mpu9250_sensor_event);
		if (motion_active) {
      if (m_framework_raw_data_func != NULL)
			  m_framework_raw_data_func(inst_accel_data, inst_gyro_data, inst_mag_data, new_timestamp);
		}
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static int
mpu9250_arch_init(mpu9250_raw_config_t *config)
{
	// initialize i2c interface
  // PRINTF("[mpu9250 arch] init \n");
  nrf_gpio_cfg_input(MPU_INT, NRF_GPIO_PIN_NOPULL);

  i2c_config_t twi;
  twi.scl = MPU_SCL;
  twi.sda = MPU_SDA;
  twi.speed = 0;
  TWI.init(&twi);

	if (config->data_source != NULL) {
	  m_framework_raw_data_func = config->data_source;
	} else {
		m_framework_raw_data_func = NULL;
	}

  gpiote_register(&gpioteh);

  mpu9250_sensor_event = process_alloc_event();
  if (!process_is_running(&mpu9250_process)) {
    process_start(&mpu9250_process, NULL);
  }
  // PRINTF("[mpu9250 arch] init completed\n");
	return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
mpu9250_arch_poweron(void)
{
  int errcode;
	if (motion_active) {
		return EINVALSTATE;
	}
  mpu_init(NULL);

	mpu_set_gyro_fsr(cfg_gyro_fsr);
	mpu_set_accel_fsr(cfg_accel_fsr);
	mpu_set_lpf(42);
	mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS);
	mpu_set_sample_rate(cfg_sample_rate);
	mpu_set_compass_sample_rate(cfg_sample_rate);
	mpu_set_int_data_ready(1);

	motion_active = true;
	gpiote_register(&gpioteh);
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
mpu9250_arch_poweroff(bool enable_wakeup_threshold)
{
  if (!motion_active)
		return EINVALSTATE;

	gpiote_unregister(&gpioteh);
	motion_active = false;
	nrf_gpio_cfg_input(MPU_INT, NRF_GPIO_PIN_NOPULL);
	mpu_set_sensors(0);

  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
mpu9250_config_update(uint32_t type, uint32_t value)
{
  if (type == DEV_MOTION_CONFIG_SAMPLEING_RATE_TYPE) {
    if (value == DEV_MOTION_RATE_DISABLE) {

    } else if (value == DEV_MOTION_RATE_10hz) {
			cfg_sample_rate = 10;
      mpu_set_sample_rate(10);
    } else if (value == DEV_MOTION_RATE_20hz) {
			cfg_sample_rate = 20;
      mpu_set_sample_rate(20);
    } else if (value == DEV_MOTION_RATE_40hz) {
			cfg_sample_rate = 40;
      mpu_set_sample_rate(40);
    } else if (value == DEV_MOTION_RATE_50hz) {
			cfg_sample_rate = 50;
      mpu_set_sample_rate(50);
    } else if (value == DEV_MOTION_RATE_100hz) {
			cfg_sample_rate = 100;
      mpu_set_sample_rate(100);
    }
  }
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static bool
mpu9250_activated(void)
{
	return motion_active;
}
/*---------------------------------------------------------------------------*/
static int
mpu9250_get_accel(short *data, uint32_t *timestamp)
{
  // values[0] = inst_accel_data[0] * MPU9250_ACCEL_COUNTS_2G;
  // values[1] = inst_accel_data[1] * MPU9250_ACCEL_COUNTS_2G;
  // values[2] = inst_accel_data[2] * MPU9250_ACCEL_COUNTS_2G;
  data[0] = inst_accel_data[0];
  data[1] = inst_accel_data[1];
  data[2] = inst_accel_data[2];
  *timestamp = new_timestamp;
	return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
mpu9250_get_gyro(short *data, uint32_t *timestamp)
{
  // values[0] = inst_gyro_data[0] * MPU9250_GYRO_COUNTS_250;
  // values[1] = inst_gyro_data[1] * MPU9250_GYRO_COUNTS_250;
  // values[2] = inst_gyro_data[2] * MPU9250_GYRO_COUNTS_250;
  data[0] = inst_gyro_data[0];
  data[1] = inst_gyro_data[1];
  data[2] = inst_gyro_data[2];
  *timestamp = new_timestamp;
	return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
mpu9250_get_compass(short *data, uint32_t *timestamp)
{
  // values[0] = inst_mag_data[0] * MPU9250_COMPASS_COUNTS_4800ut;
  // values[1] = inst_mag_data[1] * MPU9250_COMPASS_COUNTS_4800ut;
  // values[2] = inst_mag_data[2] * MPU9250_COMPASS_COUNTS_4800ut;
  data[0] = inst_mag_data[0];
  data[1] = inst_mag_data[1];
  data[2] = inst_mag_data[2];
  *timestamp = new_timestamp;
	return ENONE;
}
/*---------------------------------------------------------------------------*/
const struct mpu9250_raw_driver_impl mpu9250_raw_driver = {
	.name = "mpu9250_dmp",
	.init = mpu9250_arch_init,
	.poweron = mpu9250_arch_poweron,
	.poweroff = mpu9250_arch_poweroff,
	.config_update = mpu9250_config_update,
	.activated = mpu9250_activated,
	.get_accel = mpu9250_get_accel,
	.get_gyro = mpu9250_get_gyro,
	.get_compass = mpu9250_get_compass
};
/*---------------------------------------------------------------------------*/
