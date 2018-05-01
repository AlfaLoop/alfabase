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
#ifndef __MOTIONFUSION_SENSOR_H_
#define __MOTIONFUSION_SENSOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

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

typedef void (* motion_data_update_func_t)(uint32_t sensor_type);

typedef struct _motionfusion_config_t {
  motion_data_update_func_t        framework_raw_data_source;
} motionfusion_config_t;

typedef enum {
	X_AXIS, // 0
	Y_AXIS, // 1
	Z_AXIS  // 2
} motion_axis_order_t;

/**
 * The structure of a Motionfusion sensor in Contiki.
 */
struct motionfusion_driver {
	char *name;
	int (* init)(motionfusion_config_t *config);
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

#ifdef __cplusplus
}
#endif
#endif /* __MOTIONFUSION_SENSOR_H_ */
