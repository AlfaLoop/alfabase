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
#ifndef ___MOTION_SENSOR_H
#define ___MOTION_SENSOR_H
#ifdef __cplusplus
extern "C" {
#endif

#include "dev/sensor/motionraw-sensor.h"

int mpu9250_arch_init(motionraw_config_t *config);
int mpu9250_arch_poweron(void);
int mpu9250_arch_poweroff(bool enable_wakeup_threshold);
int mpu9250_config_update(uint32_t type, uint32_t value);
bool mpu9250_activated(void);
int mpu9250_get_accel(float *values, int32_t *data, uint32_t *timestamp);
int mpu9250_get_gyro(float *values, int32_t *data, uint32_t *timestamp);
int mpu9250_get_compass(float *values, int32_t *data, uint32_t *timestamp);

#ifdef __cplusplus
}
#endif
#endif /* ___MOTION_SENSOR_H */
