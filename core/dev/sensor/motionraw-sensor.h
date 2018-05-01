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
#ifndef __MOTIONRAW_SENSOR_H_
#define __MOTIONRAW_SENSOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

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

#define DEV_MOTION_ORIENT_PORTRAIT          0
#define DEV_MOTION_ORIENT_LANDSCAPE         1
#define DEV_MOTION_ORIENT_REVERSE_PORTRAIT  2
#define DEV_MOTION_ORIENT_REVERSE_LANDSCAPE 3


typedef void (* motion_data_update_func_t)(int16_t *accel, int16_t *gyro, int16_t *compass, uint32_t timestamp);

typedef struct _motionraw_config_t {
  motion_data_update_func_t        framework_raw_data_source;
} motionraw_config_t;

/**
 * The structure of a MotionRaw sensor in Contiki.
 */
struct motionraw_driver {
	char *name;
	int (* init)(motionraw_config_t *config);
	int (* poweron)(void);
	int (* poweroff)(bool enable_wakeup_threshold);
  int (* config_update)(uint32_t type, uint32_t value);
  bool (* activated)(void);
  int (* get_accel)(float *values, int32_t *data, uint32_t *timestamp);
  int (* get_gyro)(float *values, int32_t *data, uint32_t *timestamp);
  int (* get_compass)(float *values, int32_t *data, uint32_t *timestamp);
};

#ifdef __cplusplus
}
#endif
#endif /* __MOTIONRAW_SENSOR_H_ */
