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
#ifndef ___MPU9250_RAW_SENSOR_ARCH_H
#define ___MPU9250_RAW_SENSOR_ARCH_H
#ifdef __cplusplus
extern "C" {
#endif

#define DEV_MOTION_CONFIG_SAMPLEING_RATE_TYPE     0x00000001

#define DEV_MOTION_SCALE_2G        0x00000001
#define DEV_MOTION_SCALE_4G        0x00000002
#define DEV_MOTION_SCALE_8G        0x00000003
#define DEV_MOTION_SCALE_16G       0x00000004

#define DEV_MOTION_RATE_DISABLE     0x00000000
#define DEV_MOTION_RATE_10hz        0x00000001
#define DEV_MOTION_RATE_20hz        0x00000002
#define DEV_MOTION_RATE_40hz        0x00000003
#define DEV_MOTION_RATE_50hz        0x00000004
#define DEV_MOTION_RATE_100hz       0x00000005

/* Data ready from mpu9250 sensor. */
#define MOTIONFUSION_ACCEL                (0x01)
#define MOTIONFUSION_GYRO                 (0x02)
#define MOTIONFUSION_COMPASS              (0x04)

typedef void (* mpu9250_raw_data_update_func_t)(int16_t *accel, int16_t *gyro, int16_t *compass, uint32_t timestamp);

typedef struct _mpu9250_raw_config_t {
  mpu9250_raw_data_update_func_t   data_source;
} mpu9250_raw_config_t;

typedef enum {
	X_AXIS, // 0
	Y_AXIS, // 1
	Z_AXIS  // 2
} motion_axis_order_t;

struct mpu9250_raw_driver_impl {
	char *name;
	int (* init)(mpu9250_raw_config_t *config);
	int (* poweron)(void);
	int (* poweroff)(bool enable_wakeup_threshold);
  int (* config_update)(uint32_t type, uint32_t value);
  bool (* activated)(void);
  int (* get_accel)(short *data, uint32_t *timestamp);
  int (* get_gyro)(short *data, uint32_t *timestamp);
  int (* get_compass)(short *data, uint32_t *timestamp);
};

extern const struct mpu9250_raw_driver_impl mpu9250_dmp_driver;


#ifdef __cplusplus
}
#endif
#endif /* ___MPU9250_RAW_SENSOR_ARCH_H */
