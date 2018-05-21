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
#include "mpu9250-dmp-arch.h"
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "dev/i2c.h"
#include "dev/spi.h"
#include "frameworks/app_eventpool.h"
#include "frameworks/app_lifecycle.h"
#include "sensors/inv_mpu/inv_mpu.h"
#include "sensors/inv_mpu/inv_mpu_dmp_motion_driver.h"
#include "sensors/inv_mpu/mllite/invensense.h"
#include "sensors/inv_mpu/mpl/invensense_adv.h"
#include "sensors/inv_mpu/eMPL-hal/eMPL_outputs.h"
#include "sensors/inv_mpu/include/mltypes.h"
#include "sensors/inv_mpu/include/mpu.h"
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

/* Data read from MPL. */
#define PRINT_ACCEL     (0x01)
#define PRINT_GYRO      (0x02)
#define PRINT_QUAT      (0x04)
#define PRINT_COMPASS   (0x08)
#define PRINT_EULER     (0x10)
#define PRINT_ROT_MAT   (0x20)
#define PRINT_HEADING   (0x40)
#define PRINT_PEDO      (0x80)
#define PRINT_LINEAR_ACCEL (0x100)
#define PRINT_GRAVITY_VECTOR (0x200)

#define ACCEL_ON        (0x01)
#define GYRO_ON         (0x02)
#define COMPASS_ON      (0x04)

#define MOTION          (0)
#define NO_MOTION       (1)
#define DEFAULT_MPU_HZ  (20)

#define PEDO_READ_MS    (1000)
#define TEMP_READ_MS    (500)
#define COMPASS_READ_MS (100)

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

#define PI 3.14159265358979323846
#define MAX_DMP_SAMPLE_RATE 200           // Maximum sample rate for the DMP FIFO (200Hz)
#define FIFO_BUFFER_SIZE 512              // Max FIFO buffer size

#define MPU9250_ACCEL_COUNTS_2G     			0.00006;
#define MPU9250_ACCEL_COUNTS_4G     			0.00012;
#define MPU9250_ACCEL_COUNTS_8G     			0.00024;
#define MPU9250_ACCEL_COUNTS_16G     			0.00048;

#define MPU9250_GYRO_COUNTS_250     			0.00762;
#define MPU9250_GYRO_COUNTS_500     			0.01525;
#define MPU9250_GYRO_COUNTS_1000     			0.03051;
#define MPU9250_GYRO_COUNTS_2000     			0.06103;

#define MPU9250_COMPASS_COUNTS_4800ut     0.58593;

struct hal_s {
  unsigned char lp_accel_mode;
  unsigned char sensors;
  unsigned char dmp_on;
  unsigned char wait_for_tap;
  volatile unsigned char new_gyro;
  unsigned char motion_int_mode;
  unsigned long no_dmp_hz;
  unsigned long next_pedo_ms;
  unsigned long next_temp_ms;
  unsigned long next_compass_ms;
  unsigned short dmp_features;
};
static struct hal_s hal = {0};
unsigned char *mpl_key = (unsigned char*)"eMPL 5.1";

/* Platform-specific information. Kinda like a boardfile. */
struct platform_data_s {
    signed char orientation[9];
};

/* The sensors can be mounted onto the board in any orientation. The mounting
 * matrix seen below tells the MPL how to rotate the raw data from the
 * driver(s).
 * TODO: The following matrices refer to the configuration on internal test
 * boards at Invensense. If needed, please modify the matrices to match the
 * chip-to-body matrix for your particular set up.
 */
static struct platform_data_s gyro_pdata = {
    .orientation = { 1, 0, 0,
                     0, 1, 0,
                     0, 0, 1}
};

static struct platform_data_s compass_pdata = {
    .orientation = { 0, 1, 0,
                     1, 0, 0,
                     0, 0, -1}
};
#define COMPASS_ENABLED 1

const signed char default_orientation[9] = {
	1, 0, 0,
	0, 1, 0,
	0, 0, 1
};

struct features_s {
  float accel[3];
  int32_t accel_data[3];
  uint32_t accel_ts;

  float gyro[3];
  int32_t gyro_data[3];
  uint32_t gyro_ts;

  float compass[3];
  int32_t compass_data[3];
  uint32_t compass_ts;

  float quat[4];
  int32_t quat_data[4];
  uint32_t quat_ts;

  float euler[3];
  int32_t euler_data[3];
  uint32_t euler_ts;
  float linear_accel[3];
  uint32_t linear_accel_ts;
  float gravity[3];
  uint32_t gravity_ts;
  float heading;
  int32_t heading_data;
  uint32_t heading_ts;
};
static struct features_s features = {0};
static bool motion_active = false;
static mpu9250_dmp_data_update_func_t	m_framework_raw_data_func;
PROCESS(mpu9250_process, "Mpu9250 process");
process_event_t mpu9250_sensor_event;
/*---------------------------------------------------------------------------*/
static float
quaternion_to_float(long number, unsigned char q)
{
	unsigned long mask;
	for (int i=0; i<q; i++)
	{
		mask |= (1<<i);
	}
	return (number >> q) + ((number & mask) / (float) (2<<(q-1)));
}
/*---------------------------------------------------------------------------*/
static float
calc_quaternion(long axis)
{
	return quaternion_to_float(axis, 30);
}
/*---------------------------------------------------------------------------*/
/* Get data from MPL.
 * TODO: Add return values to the inv_get_sensor_type_xxx APIs to differentiate
 * between new and stale data.
 */
static void
read_from_mpl(void)
{
  long msg, data[9];
  int8_t accuracy;
  unsigned long timestamp;
  float float_data[3] = {0};
  uint8_t data_update = 0;

  if (!motion_active) {
    return;
  }

  if (inv_get_sensor_type_quat(data, &accuracy, (inv_time_t*)&timestamp)) {
    /* Sends a quaternion packet to the PC. Since this is used by the Python
    * test app to visually represent a 3D quaternion, it's sent each time
    * the MPL has new data.
    */
    float qw = (float)data[0];
    float qx = (float)data[1];
    float qy = (float)data[2];
    float qz = (float)data[3];

    features.quat_data[0] = (int32_t)data[0];
    features.quat_data[1] = (int32_t)data[1];
    features.quat_data[2] = (int32_t)data[2];
    features.quat_data[3] = (int32_t)data[3];
    features.quat_ts = timestamp;
    // 1073741824

    features.quat[0] = calc_quaternion(qw);
    features.quat[1] = calc_quaternion(qx);
    features.quat[2] = calc_quaternion(qy);
    features.quat[3] = calc_quaternion(qz);

    data_update |= MOTIONFUSION_QUAT;

    PRINTF("[mpu9250 arch] Quat: %f %f %f %f Time: %d ms\n",
      features.quat[0], features.quat[1], features.quat[2], features.quat[3], timestamp);
  }

  if (inv_get_sensor_type_accel(data, &accuracy, &timestamp)) {
    features.accel[0] = (int32_t)data[0] * 1.0 / 65536.0;
    features.accel[1] = (int32_t)data[1] * 1.0 / 65536.0;
    features.accel[2] = (int32_t)data[2] * 1.0 / 65536.0;
    features.accel_data[0] = (int32_t)data[0];
    features.accel_data[1] = (int32_t)data[1];
    features.accel_data[2] = (int32_t)data[2];
    features.accel_ts = timestamp;
    data_update |= MOTIONFUSION_ACCEL;
    // PRINTF("[mpu9250 arch] AccX/Y/Z: %f %f %f Time: %d ms\n", features.accel[0], features.accel[1], features.accel[2], timestamp);
    // PRINTF("[mpu9250 arch] AccX/Y/Z: %d %d %d Time: %d ms\n", features.accel_data[0], features.accel_data[1], features.accel_data[2], timestamp);
  }

  if (inv_get_sensor_type_gyro(data, &accuracy, &timestamp)) {
    features.gyro[0] = (int32_t)data[0] * 1.0 / 65536.0;
    features.gyro[1] = (int32_t)data[1] * 1.0 / 65536.0;
    features.gyro[2] = (int32_t)data[2] * 1.0 / 65536.0;
    features.gyro_data[0] = (int32_t)data[0];
    features.gyro_data[1] = (int32_t)data[1];
    features.gyro_data[2] = (int32_t)data[2];
    features.gyro_ts = timestamp;

    data_update |= MOTIONFUSION_GYRO;

    // PRINTF("[mpu9250 arch] GyroX/Y/Z: %f %f %f Time: %d ms\n",
    //     features.gyro[0], features.gyro[1], features.gyro[2], timestamp);
  }

  if (inv_get_sensor_type_compass(data, &accuracy, &timestamp)) {
    features.compass[0] = (int32_t)data[0] * 1.0 / 65536.0;
    features.compass[1] = (int32_t)data[1] * 1.0 / 65536.0;
    features.compass[2] = (int32_t)data[2] * 1.0 / 65536.0;
    features.compass_data[0] = (int32_t)data[0];
    features.compass_data[1] = (int32_t)data[1];
    features.compass_data[2] = (int32_t)data[2];
    features.compass_ts = timestamp;

    data_update |= MOTIONFUSION_COMPASS;

    // PRINTF("[mpu9250 arch] CompassX/Y/Z: %f %f %f Time: %d ms\n",
    //     features.compass[0], features.compass[1], features.compass[2], timestamp);
  }

  if (inv_get_sensor_type_euler(data, &accuracy, &timestamp)) {
    features.euler[0] = (int32_t)data[0] * 1.0 / 65536.0;  // Pitch: -180 to 180
    features.euler[1] = (int32_t)data[1] * 1.0 / 65536.0;   // Roll: -90 to 90
    features.euler[2] = (int32_t)data[2] * 1.0 / 65536.0;    // Yaw: -180 to 180
    features.euler_data[0] = (int32_t)data[0];
    features.euler_data[1] = (int32_t)data[1];
    features.euler_data[2] = (int32_t)data[2];
    features.euler_ts = timestamp;

    data_update |= MOTIONFUSION_EULER;;

    // PRINTF("[mpu9250 arch] Pitch/Roll/Yaw: %f %f %f\n",
    //     features.euler[0], features.euler[1], features.euler[2]);
  }

  if (inv_get_sensor_type_heading(data, &accuracy, &timestamp)) {
    features.heading = (int32_t)data[0] * 1.0 / 65536.0;
    features.heading_data = (int32_t)data[0];
    features.heading_ts = timestamp;

    data_update |= MOTIONFUSION_HEADING;

    // PRINTF("[mpu9250 arch] heading: %f\n", features.heading);
  }

  if (inv_get_sensor_type_linear_acceleration(float_data, &accuracy, &timestamp)) {
    features.linear_accel[0] = float_data[0];
    features.linear_accel[1] = float_data[1];
    features.linear_accel[2] = float_data[2];
    features.linear_accel_ts = timestamp;

    data_update |= MOTIONFUSION_LINEAR_ACCEL;

    // PRINTF("[mpu9250 arch] Linear Accel: %f %f %f\n", float_data[0], float_data[1], float_data[2]);
  }

  if (inv_get_sensor_type_gravity(float_data, &accuracy, &timestamp)) {
    features.gravity[0] = float_data[0];
    features.gravity[1] = float_data[2];
    features.gravity[2] = float_data[2];
    features.gravity_ts = timestamp;
    data_update |= MOTIONFUSION_GRAVITY_VECTOR;
    // PRINTF("[mpu9250 arch] Gravity Vector: %f %f %f\n",
    //     float_data[0], float_data[1], float_data[2]);
  }

  timestamp = clock_time();
  if (timestamp > hal.next_pedo_ms) {
    hal.next_pedo_ms = timestamp + PEDO_READ_MS;
    unsigned long step_count, walk_time;
    dmp_get_pedometer_step_count(&step_count);
    dmp_get_pedometer_walk_time(&walk_time);
    // PRINTF("[mpu9250 arch] Walked %ld steps over %ld ms\n", step_count, walk_time);
  }

  if (data_update) {
    if (m_framework_raw_data_func != NULL) {
      m_framework_raw_data_func(data_update);
    }
  }

}
/*---------------------------------------------------------------------------*/
static void
mpu9250_int_event_handler(gpiote_event_t *event)
{
  uint32_t pin_mask = (1u << MPU_INT);
  if (event->pin_no == pin_mask) {
    if (motion_active) {
      if (hal.motion_int_mode) {
        /* Restore the previous sensor configuration. */
        mpu_lp_motion_interrupt(0, 0, 0);
        hal.motion_int_mode = 0;
        process_post(&mpu9250_process, mpu9250_sensor_event, NULL);
      } else {
        hal.new_gyro = 1;
        process_post(&mpu9250_process, mpu9250_sensor_event, NULL);
      }
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
static void
tap_cb(unsigned char direction, unsigned char count)
{
  switch (direction) {
  case TAP_X_UP:
    PRINTF("[mpu9250 arch] Tap X+\n");
    break;
  case TAP_X_DOWN:
    PRINTF("[mpu9250 arch] Tap X- \n");
    break;
  case TAP_Y_UP:
    PRINTF("[mpu9250 arch] Tap Y+\n");
    break;
  case TAP_Y_DOWN:
    PRINTF("[mpu9250 arch] Tap Y-\n");
    break;
  case TAP_Z_UP:
    PRINTF("[mpu9250 arch] Tap Z+\n");
    break;
  case TAP_Z_DOWN:
    PRINTF("[mpu9250 arch] Tap Z-\n");
    break;
  default:
    return;
  }
  PRINTF("[mpu9250 arch] tap_cb x%d\n", count);
  return;
}
/*---------------------------------------------------------------------------*/
static void
android_orient_cb(unsigned char orientation)
{
	switch (orientation) {
	case ANDROID_ORIENT_PORTRAIT:
    PRINTF("[mpu9250 arch] Portrait\n");
    break;
	case ANDROID_ORIENT_LANDSCAPE:
    PRINTF("[mpu9250 arch] Landscape\n");
    break;
	case ANDROID_ORIENT_REVERSE_PORTRAIT:
    PRINTF("[mpu9250 arch] Reverse Portrait\n");
    break;
	case ANDROID_ORIENT_REVERSE_LANDSCAPE:
    PRINTF("[mpu9250 arch] Reverse Landscape\n");
    break;
	default:
		return;
	}
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mpu9250_process, ev, data)
{
  static int errcode;
  static uint8_t new_temp = 0, new_compass = 0;
  static uint32_t timestamp;
  static uint32_t sensor_timestamp;
  static int new_data = 0;
  static uint8_t accel_fsr;
  static uint16_t gyro_rate, gyro_fsr, compass_fsr;

	PROCESS_BEGIN();

	mpu_init(NULL);
	/* If you're not using an MPU9150 AND you're not using DMP features, this
		* function will place all slaves on the primary bus.
		* mpu_set_bypass(1);
	*/
	errcode = inv_init_mpl();
	if (errcode) {
		PRINTF("[mpu9250 arch] Could not initialize MPL.\n");
	}

	// Compute 6-axis and 9-axis quaternions.
	inv_enable_quaternion();
	inv_enable_9x_sensor_fusion();

	// MPL will depend on the timestamps passed to inv_build_compass instead.
	// inv_9x_fusion_use_timestamps(1);

	/* Update gyro biases when not in motion.
		* WARNING: These algorithms are mutually exclusive.
		*/
	inv_enable_fast_nomot();

	/* Update gyro biases when temperature changes. */
	inv_enable_gyro_tc();

	/* Compass calibration algorithms. */
	inv_enable_vector_compass_cal();
	inv_enable_magnetic_disturbance();

  inv_enable_eMPL_outputs();

	errcode = inv_start_mpl();
	if (errcode == INV_ERROR_NOT_AUTHORIZED) {
		while (1) {
			PRINTF("[mpu9250 arch] Not authorized.\n");
		}
	}
	if (errcode) {
		PRINTF("Could not start the MPL.\n");
	}

	mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS);

	// Push both gyro and accel data into the FIFO.
	mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
	mpu_set_sample_rate(DEFAULT_MPU_HZ);

	/* The compass sampling rate can be less than the gyro/accel sampling rate.
	 * Use this function for proper power management.
	 */
	mpu_set_compass_sample_rate(1000 / COMPASS_READ_MS);

	// Read back configuration in case it was set improperly.
	mpu_get_sample_rate(&gyro_rate);
	mpu_get_gyro_fsr(&gyro_fsr);
	mpu_get_accel_fsr(&accel_fsr);
	mpu_get_compass_fsr(&compass_fsr);

	/* Sync driver configuration with MPL. */
	/* Sample rate expected in microseconds. */
	inv_set_gyro_sample_rate(1000000L / gyro_rate);
	inv_set_accel_sample_rate(1000000L / gyro_rate);

	/* The compass rate is independent of the gyro and accel rates. As long as
   * inv_set_compass_sample_rate is called with the correct value, the 9-axis
   * fusion algorithm's compass correction gain will work properly.
   */
	inv_set_compass_sample_rate(COMPASS_READ_MS * 1000L);

  /* Set chip-to-body orientation matrix.
    * Set hardware units to dps/g's/degrees scaling factor.
    */
  inv_set_gyro_orientation_and_scale(
         inv_orientation_matrix_to_scalar(gyro_pdata.orientation),
         (long)gyro_fsr<<15);
  inv_set_accel_orientation_and_scale(
         inv_orientation_matrix_to_scalar(gyro_pdata.orientation),
         (long)accel_fsr<<15);
  inv_set_compass_orientation_and_scale(
         inv_orientation_matrix_to_scalar(compass_pdata.orientation),
         (long)compass_fsr<<15);

  hal.sensors = ACCEL_ON | GYRO_ON | COMPASS_ON;
  hal.dmp_on = 0;
  hal.next_pedo_ms = 0;
  hal.next_compass_ms = 0;
  hal.next_temp_ms = 0;

  timestamp = clock_time();

  /* To initialize the DMP:
     * 1. Call dmp_load_motion_driver_firmware(). This pushes the DMP image in
     *    inv_mpu_dmp_motion_driver.h into the MPU memory.
     * 2. Push the gyro and accel orientation matrix to the DMP.
     * 3. Register gesture callbacks. Don't worry, these callbacks won't be
     *    executed unless the corresponding feature is enabled.
     * 4. Call dmp_enable_feature(mask) to enable different features.
     * 5. Call dmp_set_fifo_rate(freq) to select a DMP output rate.
     * 6. Call any feature-specific control functions.
     *
     * To enable the DMP, just call mpu_set_dmp_state(1). This function can
     * be called repeatedly to enable and disable the DMP at runtime.
     *
     * The following is a short summary of the features supported in the DMP
     * image provided in inv_mpu_dmp_motion_driver.c:
     * DMP_FEATURE_LP_QUAT: Generate a gyro-only quaternion on the DMP at
     * 200Hz. Integrating the gyro data at higher rates reduces numerical
     * errors (compared to integration on the MCU at a lower sampling rate).
     * DMP_FEATURE_6X_LP_QUAT: Generate a gyro/accel quaternion on the DMP at
     * 200Hz. Cannot be used in combination with DMP_FEATURE_LP_QUAT.
     * DMP_FEATURE_TAP: Detect taps along the X, Y, and Z axes.
     * DMP_FEATURE_ANDROID_ORIENT: Google's screen rotation algorithm. Triggers
     * an event at the four orientations where the screen should rotate.
     * DMP_FEATURE_GYRO_CAL: Calibrates the gyro data after eight seconds of
     * no motion.
     * DMP_FEATURE_SEND_RAW_ACCEL: Add raw accelerometer data to the FIFO.
     * DMP_FEATURE_SEND_RAW_GYRO: Add raw gyro data to the FIFO.
     * DMP_FEATURE_SEND_CAL_GYRO: Add calibrated gyro data to the FIFO. Cannot
     * be used in combination with DMP_FEATURE_SEND_RAW_GYRO.
     */
  dmp_load_motion_driver_firmware();
  dmp_set_orientation(
      inv_orientation_matrix_to_scalar(gyro_pdata.orientation));
  dmp_register_tap_cb(tap_cb);
  dmp_register_android_orient_cb(android_orient_cb);

  hal.dmp_features = DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_TAP |
        DMP_FEATURE_ANDROID_ORIENT | DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_CAL_GYRO |
        DMP_FEATURE_GYRO_CAL;
  dmp_enable_feature(hal.dmp_features);
  dmp_set_fifo_rate(DEFAULT_MPU_HZ);
  unsigned short sample_rate;
  mpu_get_sample_rate(&sample_rate);
  inv_set_quat_sample_rate(1000000L / sample_rate);
  mpu_set_dmp_state(1);
  hal.dmp_on = 1;
  motion_active = true;

  /* Reset pedometer. */
  dmp_set_pedometer_step_count(0);
  dmp_set_pedometer_walk_time(0);

	while(1) {
		PROCESS_WAIT_EVENT_UNTIL(ev == mpu9250_sensor_event);
    do {

      timestamp = clock_time();

      /* We're not using a data ready interrupt for the compass, so we'll
       * make our compass reads timer-based instead.
      */
      if ((timestamp > hal.next_compass_ms) && !hal.lp_accel_mode &&
        hal.new_gyro && (hal.sensors & COMPASS_ON)) {
        hal.next_compass_ms = timestamp + COMPASS_READ_MS;
        new_compass = 1;
      }

      /* Temperature data doesn't need to be read with every gyro sample.
       * Let's make them timer-based like the compass reads.
      */
      if (timestamp > hal.next_temp_ms) {
        hal.next_temp_ms = timestamp + TEMP_READ_MS;
        new_temp = 1;
      }

      if (hal.motion_int_mode) {
        /* Enable motion interrupt. */
        mpu_lp_motion_interrupt(500, 1, 5);
        /* Notify the MPL that contiguity was broken. */
        inv_accel_was_turned_off();
        inv_gyro_was_turned_off();
        inv_compass_was_turned_off();
        inv_quaternion_sensor_was_turned_off();
        /* Wait for the MPU interrupt. */
        PROCESS_WAIT_EVENT_UNTIL(ev == mpu9250_sensor_event);
        /* Restore the previous sensor configuration. */
        mpu_lp_motion_interrupt(0, 0, 0);
        hal.motion_int_mode = 0;
      }

      if (hal.new_gyro && hal.lp_accel_mode) {
        short accel_short[3];
        long accel[3];
        mpu_get_accel_reg(accel_short, &sensor_timestamp);
        accel[0] = (long)accel_short[0];
        accel[1] = (long)accel_short[1];
        accel[2] = (long)accel_short[2];
        inv_build_accel(accel, 0, sensor_timestamp);
        new_data = 1;
        hal.new_gyro = 0;
      } else if (hal.new_gyro && hal.dmp_on) {
        short gyro[3], accel_short[3], sensors;
        unsigned char more;
        long accel[3], quat[4], temperature;
        /* This function gets new data from the FIFO when the DMP is in
         * use. The FIFO can contain any combination of gyro, accel,
         * quaternion, and gesture data. The sensors parameter tells the
         * caller which data fields were actually populated with new data.
         * For example, if sensors == (INV_XYZ_GYRO | INV_WXYZ_QUAT), then
         * the FIFO isn't being filled with accel data.
         * The driver parses the gesture data to determine if a gesture
         * event has occurred; on an event, the application will be notified
         * via a callback (assuming that a callback function was properly
         * registered). The more parameter is non-zero if there are
         * leftover packets in the FIFO.
         */
        dmp_read_fifo(gyro, accel_short, quat, &sensor_timestamp, &sensors, &more);
        if (!more)
            hal.new_gyro = 0;
        if (sensors & INV_XYZ_GYRO) {
          /* Push the new data to the MPL. */
          inv_build_gyro(gyro, sensor_timestamp);
          new_data = 1;
          if (new_temp) {
            new_temp = 0;
            /* Temperature only used for gyro temp comp. */
            mpu_get_temperature(&temperature, &sensor_timestamp);
            inv_build_temp(temperature, sensor_timestamp);
          }
        }
        if (sensors & INV_XYZ_ACCEL) {
          accel[0] = (long)accel_short[0];
          accel[1] = (long)accel_short[1];
          accel[2] = (long)accel_short[2];
          inv_build_accel(accel, 0, sensor_timestamp);
          new_data = 1;
        }
        if (sensors & INV_WXYZ_QUAT) {
          inv_build_quat(quat, 0, sensor_timestamp);
          new_data = 1;
        }
      } else if (hal.new_gyro) {
        short gyro[3], accel_short[3];
        unsigned char sensors, more;
        long accel[3], temperature;
        /* This function gets new data from the FIFO. The FIFO can contain
         * gyro, accel, both, or neither. The sensors parameter tells the
         * caller which data fields were actually populated with new data.
         * For example, if sensors == INV_XYZ_GYRO, then the FIFO isn't
         * being filled with accel data. The more parameter is non-zero if
         * there are leftover packets in the FIFO. The HAL can use this
         * information to increase the frequency at which this function is
         * called.
         */
        hal.new_gyro = 0;
        mpu_read_fifo(gyro, accel_short, &sensor_timestamp, &sensors, &more);
        if (more)
          hal.new_gyro = 1;
        if (sensors & INV_XYZ_GYRO) {
          /* Push the new data to the MPL. */
          inv_build_gyro(gyro, sensor_timestamp);
          new_data = 1;
          if (new_temp) {
            new_temp = 0;
            /* Temperature only used for gyro temp comp. */
            mpu_get_temperature(&temperature, &sensor_timestamp);
            inv_build_temp(temperature, sensor_timestamp);
          }
        }
        if (sensors & INV_XYZ_ACCEL) {
          accel[0] = (long)accel_short[0];
          accel[1] = (long)accel_short[1];
          accel[2] = (long)accel_short[2];
          inv_build_accel(accel, 0, sensor_timestamp);
          new_data = 1;
        }
      }

      if (new_compass) {
        short compass_short[3];
        long compass[3];
        new_compass = 0;
        /* For any MPU device with an AKM on the auxiliary I2C bus, the raw
         * magnetometer registers are copied to special gyro registers.
         */
        if (!mpu_get_compass_reg(compass_short, &sensor_timestamp)) {
            compass[0] = (long)compass_short[0];
            compass[1] = (long)compass_short[1];
            compass[2] = (long)compass_short[2];
            /* NOTE: If using a third-party compass calibration library,
             * pass in the compass data in uT * 2^16 and set the second
             * parameter to INV_CALIBRATED | acc, where acc is the
             * accuracy from 0 to 3.
             */
            inv_build_compass(compass, 0, sensor_timestamp);
        }
        new_data = 1;
      }

      if (new_data) {
        // PRINTF("[mpu9250 proc] new data \n");
        inv_execute_on_data();
        /* This function reads bias-compensated sensor data and sensor
         * fusion outputs from the MPL. The outputs are formatted as seen
         * in eMPL_outputs.c. This function only needs to be called at the
         * rate requested by the host.
         */
        read_from_mpl();
      }

    } while (hal.new_gyro);

	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static int
mpu9250_arch_init(mpu9250_dmp_config_t *config)
{
	// initialize i2c interface
  nrf_gpio_cfg_input(MPU_INT, NRF_GPIO_PIN_NOPULL);

  nrf_gpio_cfg_output(MPU_CS);
  nrf_gpio_pin_set(MPU_CS);

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
  // mpu_init(NULL);
  // mpu_set_sensors(0);
	return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
mpu9250_arch_poweron(void)
{
  int errcode;
  unsigned char mask = 0, lp_accel_was_on = 0;
  unsigned short sample_rate;

	if (motion_active) {
		return EINVALSTATE;
	}

  hal.sensors = ACCEL_ON | GYRO_ON | COMPASS_ON;
  mask |= INV_XYZ_ACCEL;
  mask |= INV_XYZ_GYRO;
  lp_accel_was_on |= hal.lp_accel_mode;
  mask |= INV_XYZ_COMPASS;
  lp_accel_was_on |= hal.lp_accel_mode;

  /* If you need a power transition, this function should be called with a
   * mask of the sensors still enabled. The driver turns off any sensors
   * excluded from this mask.
   */
  mpu_set_sensors(mask);
  mpu_configure_fifo(mask);
  if (lp_accel_was_on) {
    unsigned short rate;
    hal.lp_accel_mode = 0;
    /* Switching out of LP accel, notify MPL of new accel sampling rate. */
    mpu_get_sample_rate(&rate);
    inv_set_accel_sample_rate(1000000L / rate);
  }

  hal.dmp_on = 1;
  /* Preserve current FIFO rate. */
  mpu_get_sample_rate(&sample_rate);
  dmp_set_fifo_rate(sample_rate);
  inv_set_quat_sample_rate(1000000L / sample_rate);
  mpu_set_dmp_state(1);
  PRINTF("[mpu9250 arch] DMP enabled.\n");

	motion_active = true;
}
/*---------------------------------------------------------------------------*/
static int
mpu9250_arch_poweroff(bool enable_wakeup_threshold)
{
  if (!motion_active)
		return EINVALSTATE;

  if (enable_wakeup_threshold) {
    hal.motion_int_mode = 1;

    /* System Off? */
  } else {
    // Disable DMP and Enable LP accel mode
    unsigned short dmp_rate;
    unsigned char mask = 0;
    hal.dmp_on = 0;
    mpu_set_dmp_state(0);
    /* Restore FIFO settings. */
    if (hal.sensors & ACCEL_ON)
        mask |= INV_XYZ_ACCEL;
    if (hal.sensors & GYRO_ON)
        mask |= INV_XYZ_GYRO;
    if (hal.sensors & COMPASS_ON)
        mask |= INV_XYZ_COMPASS;
    mpu_configure_fifo(mask);
    /* When the DMP is used, the hardware sampling rate is fixed at
     * 200Hz, and the DMP is configured to downsample the FIFO output
     * using the function dmp_set_fifo_rate. However, when the DMP is
     * turned off, the sampling rate remains at 200Hz. This could be
     * handled in inv_mpu.c, but it would need to know that
     * inv_mpu_dmp_motion_driver.c exists. To avoid this, we'll just
     * put the extra logic in the application layer.
     */
    dmp_get_fifo_rate(&dmp_rate);
    mpu_set_sample_rate(dmp_rate);
    inv_quaternion_sensor_was_turned_off();
    PRINTF("[mpu9250 arch] DMP disabled.\n");

    mpu_lp_accel_mode(1);
    /* When LP accel mode is enabled, the driver automatically configures
     * the hardware for latched interrupts. However, the MCU sometimes
     * misses the rising/falling edge, and the hal.new_gyro flag is never
     * set. To avoid getting locked in this state, we're overriding the
     * driver's configuration and sticking to unlatched interrupt mode.
     *
     * TODO: The MCU supports level-triggered interrupts.
     */
    mpu_set_int_latched(0);
    hal.sensors &= ~(GYRO_ON|COMPASS_ON);
    hal.sensors |= ACCEL_ON;
    hal.lp_accel_mode = 1;
    inv_gyro_was_turned_off();
    inv_compass_was_turned_off();
  }

	// gpiote_unregister(&gpioteh);
	motion_active = false;
	// nrf_gpio_cfg_input(MPU_INT, NRF_GPIO_PIN_NOPULL);
	// mpu_set_sensors(0);
}
/*---------------------------------------------------------------------------*/
static int
mpu9250_config_update(uint32_t type, uint32_t value)
{
  if (type == DEV_MOTION_CONFIG_SAMPLEING_RATE_TYPE) {
    if (value == DEV_MOTION_RATE_DISABLE) {

    } else if (value == DEV_MOTION_RATE_10hz) {
      if (hal.dmp_on) {
          dmp_set_fifo_rate(10);
          inv_set_quat_sample_rate(100000L);
      } else {
        mpu_set_sample_rate(10);
      }
      inv_set_gyro_sample_rate(100000L);
      inv_set_accel_sample_rate(100000L);
    } else if (value == DEV_MOTION_RATE_20hz) {
      if (hal.dmp_on) {
        dmp_set_fifo_rate(20);
        inv_set_quat_sample_rate(50000L);
      } else {
        mpu_set_sample_rate(20);
      }
      inv_set_gyro_sample_rate(50000L);
      inv_set_accel_sample_rate(50000L);
    } else if (value == DEV_MOTION_RATE_40hz) {
      if (hal.dmp_on) {
        dmp_set_fifo_rate(40);
        inv_set_quat_sample_rate(25000L);
      } else {
        mpu_set_sample_rate(40);
      }
      inv_set_gyro_sample_rate(25000L);
      inv_set_accel_sample_rate(25000L);
    } else if (value == DEV_MOTION_RATE_50hz) {
      if (hal.dmp_on) {
        dmp_set_fifo_rate(50);
        inv_set_quat_sample_rate(20000L);
      } else {
        mpu_set_sample_rate(50);
      }
      inv_set_gyro_sample_rate(20000L);
      inv_set_accel_sample_rate(20000L);
    } else if (value == DEV_MOTION_RATE_100hz) {
      if (hal.dmp_on) {
          dmp_set_fifo_rate(100);
          inv_set_quat_sample_rate(10000L);
      } else {
        mpu_set_sample_rate(100);
      }
      inv_set_gyro_sample_rate(10000L);
      inv_set_accel_sample_rate(10000L);
    } else {

    }
  } else if (type == DEV_MOTION_CONFIG_RESET_PEDOMETER_TYPE) {
    /* Reset pedometer. */
    dmp_set_pedometer_step_count(0);
    dmp_set_pedometer_walk_time(0);
  }
}
/*---------------------------------------------------------------------------*/
static bool
mpu9250_activated(void)
{
	return motion_active;
}
/*---------------------------------------------------------------------------*/
static int
mpu9250_get_accel(float *values, int32_t *data, uint32_t *timestamp)
{
  values[0] = features.accel[0];
  values[1] = features.accel[1];
  values[2] = features.accel[2];
  data[0] = features.accel_data[0];
  data[1] = features.accel_data[1];
  data[2] = features.accel_data[2];
  *timestamp = features.accel_ts;
	return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
mpu9250_get_gyro(float *values, int32_t *data, uint32_t *timestamp)
{
  values[0] = features.gyro[0];
  values[1] = features.gyro[1];
  values[2] = features.gyro[2];
  data[0] = features.gyro_data[0];
  data[1] = features.gyro_data[1];
  data[2] = features.gyro_data[2];
  *timestamp = features.gyro_ts;
	return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
mpu9250_get_compass(float *values, int32_t *data, uint32_t *timestamp)
{
  values[0] = features.compass[0];
  values[1] = features.compass[1];
  values[2] = features.compass[2];
  data[0] = features.compass_data[0];
  data[1] = features.compass_data[1];
  data[2] = features.compass_data[2];
  *timestamp = features.compass_ts;
	return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
mpu9250_get_quaternion(float *values, int32_t *data, uint32_t *timestamp)
{
  values[0] = features.quat[0];
  values[1] = features.quat[1];
  values[2] = features.quat[2];
  values[3] = features.quat[3];
  data[0] = features.quat_data[0];
  data[1] = features.quat_data[1];
  data[2] = features.quat_data[2];
  data[3] = features.quat_data[3];

  *timestamp = features.quat_ts;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
mpu9250_get_euler(float *values, int32_t *data, uint32_t *timestamp)
{
  values[0] = features.euler[0];
  values[1] = features.euler[1];
  values[2] = features.euler[2];
  data[0] = features.euler_data[0];
  data[1] = features.euler_data[1];
  data[2] = features.euler_data[2];

  *timestamp = features.euler_ts;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
mpu9250_get_linear_accel(float *values, uint32_t *timestamp)
{
  values[0] = features.linear_accel[0];
  values[1] = features.linear_accel[1];
  values[2] = features.linear_accel[2];

  *timestamp = features.linear_accel_ts;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
mpu9250_get_gravity_vector(float *values, uint32_t *timestamp)
{
  values[0] = features.gravity[0];
  values[1] = features.gravity[1];
  values[2] = features.gravity[2];

  *timestamp = features.gravity_ts;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
mpu9250_get_heading(float *value, int32_t *data, uint32_t *timestamp)
{
  *value = features.heading;
  *data = features.heading_data;
  *timestamp = features.heading_ts;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
const struct mpu9250_dmp_driver_impl mpu9250_dmp_driver = {
	.name = "mpu9250_dmp",
	.init = mpu9250_arch_init,
	.poweron = mpu9250_arch_poweron,
	.poweroff = mpu9250_arch_poweroff,
	.config_update = mpu9250_config_update,
	.activated = mpu9250_activated,
	.get_accel = mpu9250_get_accel,
	.get_gyro = mpu9250_get_gyro,
	.get_compass = mpu9250_get_compass,
	.get_quaternion = mpu9250_get_quaternion,
	.get_euler = mpu9250_get_euler,
	.get_linear_accel = mpu9250_get_linear_accel,
	.get_gravity_vector = mpu9250_get_gravity_vector,
	.get_heading = mpu9250_get_heading
};
/*---------------------------------------------------------------------------*/
