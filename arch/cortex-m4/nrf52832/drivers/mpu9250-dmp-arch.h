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
#ifndef ___MPU9250_DMP_SENSOR_ARCH_H
#define ___MPU9250_DMP_SENSOR_ARCH_H
#ifdef __cplusplus
extern "C" {
#endif

#define DEV_MOTION_CONFIG_SAMPLEING_RATE_TYPE     0x00000001
#define DEV_MOTION_CONFIG_RESET_PEDOMETER_TYPE    0x00000002

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

#define DEV_MOTION_ORIENT_PORTRAIT          0
#define DEV_MOTION_ORIENT_LANDSCAPE         1
#define DEV_MOTION_ORIENT_REVERSE_PORTRAIT  2
#define DEV_MOTION_ORIENT_REVERSE_LANDSCAPE 3

/* Data ready from motion sensor. */
#define MOTIONFUSION_ACCEL                (0x01)
#define MOTIONFUSION_GYRO                 (0x02)
#define MOTIONFUSION_COMPASS              (0x04)
#define MOTIONFUSION_QUAT                 (0x08)
#define MOTIONFUSION_EULER                (0x10)
#define MOTIONFUSION_HEADING              (0x20)
#define MOTIONFUSION_LINEAR_ACCEL         (0x40)
#define MOTIONFUSION_GRAVITY_VECTOR       (0x80)

typedef void (* mpu9250_dmp_data_update_func_t)(uint8_t sensor_type);

typedef struct _mpu9250_dmp_config_t {
  mpu9250_dmp_data_update_func_t   data_source;
} mpu9250_dmp_config_t;

typedef enum {
	X_AXIS, // 0
	Y_AXIS, // 1
	Z_AXIS  // 2
} motion_axis_order_t;

struct mpu9250_dmp_driver_impl {
	char *name;
	int (* init)(mpu9250_dmp_config_t *config);
	int (* poweron)(void);
	int (* poweroff)(bool enable_wakeup_threshold);
  int (* config_update)(uint32_t type, uint32_t value);
  bool (* activated)(void);
  int (* get_accel)(float *values, int32_t *data, uint32_t *timestamp);
  int (* get_gyro)(float *values, int32_t *data, uint32_t *timestamp);
  int (* get_compass)(float *values, int32_t *data, uint32_t *timestamp);
  int (* get_quaternion)(float *values, int32_t *data, uint32_t *timestamp);
  int (* get_euler)(float *values, int32_t *data, uint32_t *timestamp);
  int (* get_linear_accel)(float *values, uint32_t *timestamp);
  int (* get_gravity_vector)(float *values, uint32_t *timestamp);
  int (* get_heading)(float *value, int32_t *data, uint32_t *timestamp);
};

extern const struct mpu9250_dmp_driver_impl mpu9250_dmp_driver;


#ifdef __cplusplus
}
#endif
#endif /* ___MPU9250_DMP_SENSOR_ARCH_H */
