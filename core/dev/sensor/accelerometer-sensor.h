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
#ifndef __ACCELEROMETER_SENSOR_H_
#define __ACCELEROMETER_SENSOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define DEV_ACCEL_SCALE_2G        0x00000001
#define DEV_ACCEL_SCALE_4G        0x00000002
#define DEV_ACCEL_SCALE_8G        0x00000003
#define DEV_ACCEL_SCALE_16G       0x00000004

#define DEV_ACCEL_RATE_10hz        0x00000001
#define DEV_ACCEL_RATE_20hz        0x00000002
#define DEV_ACCEL_RATE_50hz        0x00000003
#define DEV_ACCEL_RATE_100hz       0x00000004

typedef void (* accel_data_func_t)(float x, float y, float z, uint32_t timestamp);
typedef void (* accel_raw_data_func_t)(int16_t raw_x, int16_t raw_y, int16_t raw_z, float x, float y, float z, uint32_t timestamp);
typedef void (* accel_event_changed_func_t)(uint32_t type);

typedef struct _accelerometer_config_t {
  accel_raw_data_func_t        framework_raw_data_source;
  accel_data_func_t        pedometer_raw_data_source;
  uint32_t    range;
  uint32_t    rate;
} accelerometer_config_t;

/**
 * The structure of a Accelerometer sensor in Contiki.
 */
struct accelerometer_driver {
	char *name;
	int (* init)(accelerometer_config_t *config);
	int (* poweron)(void);
	int (* poweroff)(bool enable_wakeup_threshold);
  int (* config_update)(uint32_t type, uint32_t value);
  bool (* activated)(void);
  float (* getx)(void);
  float (* gety)(void);
  float (* getz)(void);
};

extern const struct accelerometer_driver SENSOR_ACCEL;


#ifdef __cplusplus
}
#endif
#endif /* __ACCELEROMETER_SENSOR_H_ */
