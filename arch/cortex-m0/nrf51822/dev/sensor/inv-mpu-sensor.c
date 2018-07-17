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
#include "contiki.h"
#include "inv-mpu-sensor.h"
#include <stdint.h>
#include <stdbool.h>
#include "dev/i2c.h"
#include "dev/spi.h"
#include "dev/sensors.h"
#include "dev/power.h"
#include "frameworks/app_eventpool.h"
#include "frameworks/app_lifecycle.h"
#include "dev/sensor/inv_mpu/inv_mpu.h"
#include "gpiote.h"
#include "nrf_gpio.h"
#include "sys/clock.h"
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
#define DEFAULT_MPU_HZ  (50)
#ifndef MPU_INT
#define MPU_INT 19U
#endif /* MPU_INT */
/*---------------------------------------------------------------------------*/
#define MPU9250_ACCEL_COUNTS_2G     			0.00006;
#define MPU9250_ACCEL_COUNTS_4G     			0.00012;
#define MPU9250_ACCEL_COUNTS_8G     			0.00024;
#define MPU9250_ACCEL_COUNTS_16G     			0.00048;

#define MPU9250_GYRO_COUNTS_250     			0.00762;
#define MPU9250_GYRO_COUNTS_500     			0.01525;
#define MPU9250_GYRO_COUNTS_1000     			0.03051;
#define MPU9250_GYRO_COUNTS_2000     			0.06103;

#define MPU9250_COMPASS_COUNTS_4800ut     0.58593;

#define TWI INV_MPU_TWI_DRIVER_CONF
extern const struct i2c_driver TWI;
static bool 	 motion_active = false;
static uint8_t   cfg_sample_rate = 50;
static unsigned short  cfg_gyro_fsr = 250;
static unsigned short  cfg_accel_fsr = 2;
static short     inst_accel_data[3];
static short     inst_gyro_data[3];
static short     inst_mag_data[3];
static uint32_t  new_timestamp;
static motion_data_update_func_t 	m_framework_raw_data_func;
PROCESS(mpu9250_process, "Mpu9250 process");
process_event_t mpu9250_sensor_event;
/*---------------------------------------------------------------------------*/
static void
motion_int_event_handler(gpiote_event_t *event)
{
	uint32_t pin_mask = (1u << MPU_INT);
  if (event->pin_no == pin_mask) {
    if (motion_active) {
			mpu_get_gyro_reg(inst_gyro_data, NULL);
			mpu_get_accel_reg(inst_accel_data, NULL);
			mpu_get_compass_reg(inst_mag_data, NULL);
			new_timestamp = clock_time();
			process_post(&mpu9250_process, mpu9250_sensor_event, NULL);
    }
  }
}
/*---------------------------------------------------------------------------*/
static gpiote_handle_t gpioteh = {.event_handler=motion_int_event_handler,
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
			m_framework_raw_data_func(inst_accel_data, inst_gyro_data, inst_mag_data, new_timestamp);
		}
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
int
mpu9250_arch_init(motionraw_config_t *config)
{
	// initialize i2c interface
  nrf_gpio_cfg_input(MPU_INT, NRF_GPIO_PIN_NOPULL);

  i2c_config_t twi;
  twi.scl = MPU_SCL;
  twi.sda = MPU_SDA;
  twi.speed = 0;
  TWI.init(&twi);

	if (config->framework_raw_data_source != NULL) {
	  m_framework_raw_data_func = config->framework_raw_data_source;
	} else {
		m_framework_raw_data_func = NULL;
	}

  gpiote_register(&gpioteh);

  mpu9250_sensor_event = process_alloc_event();
  if (!process_is_running(&mpu9250_process)) {
    process_start(&mpu9250_process, NULL);
  }
	return ENONE;
}
/*---------------------------------------------------------------------------*/
int
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
int
mpu9250_arch_poweroff(bool enable_wakeup_threshold)
{
	if (!motion_active)
		return EINVALSTATE;

	gpiote_unregister(&gpioteh);
	motion_active = false;
	nrf_gpio_cfg_input(MPU_INT, NRF_GPIO_PIN_NOPULL);
	mpu_set_sensors(0);
}
/*---------------------------------------------------------------------------*/
int
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
}
/*---------------------------------------------------------------------------*/
bool
mpu9250_activated(void)
{
	return motion_active;
}
/*---------------------------------------------------------------------------*/
int
mpu9250_get_accel(float *values, int32_t *data, uint32_t *timestamp)
{
  values[0] = inst_accel_data[0] * MPU9250_ACCEL_COUNTS_2G;
  values[1] = inst_accel_data[1] * MPU9250_ACCEL_COUNTS_2G;
  values[2] = inst_accel_data[2] * MPU9250_ACCEL_COUNTS_2G;
  data[0] = inst_accel_data[0];
  data[1] = inst_accel_data[1];
  data[2] = inst_accel_data[2];
  *timestamp = new_timestamp;
	return ENONE;
}
/*---------------------------------------------------------------------------*/
int
mpu9250_get_gyro(float *values, int32_t *data, uint32_t *timestamp)
{
  values[0] = inst_gyro_data[0] * MPU9250_GYRO_COUNTS_250;
  values[1] = inst_gyro_data[1] * MPU9250_GYRO_COUNTS_250;
  values[2] = inst_gyro_data[2] * MPU9250_GYRO_COUNTS_250;
  data[0] = inst_gyro_data[0];
  data[1] = inst_gyro_data[1];
  data[2] = inst_gyro_data[2];
  *timestamp = new_timestamp;
	return ENONE;
}
/*---------------------------------------------------------------------------*/
int
mpu9250_get_compass(float *values, int32_t *data, uint32_t *timestamp)
{
  values[0] = inst_mag_data[0] * MPU9250_COMPASS_COUNTS_4800ut;
  values[1] = inst_mag_data[1] * MPU9250_COMPASS_COUNTS_4800ut;
  values[2] = inst_mag_data[2] * MPU9250_COMPASS_COUNTS_4800ut;
  data[0] = inst_mag_data[0];
  data[1] = inst_mag_data[1];
  data[2] = inst_mag_data[2];
  *timestamp = new_timestamp;
	return ENONE;
}
/*---------------------------------------------------------------------------*/
