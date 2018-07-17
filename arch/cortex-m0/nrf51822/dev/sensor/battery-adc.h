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
#ifndef _BATTERY_ADC_H
#define _BATTERY_ADC_H
#ifdef __cplusplus
extern "C" {
#endif

#include "frameworks/sensors/sensor_api.h"

#define BATTERY_ADC_SAMPLING_SOURCE_INTERNAL      1
#define BATTERY_ADC_SAMPLING_SOURCE_EXT           2

int sensor_battery_arch_init(void);

bool battery_ossensor_is_supported(void);
int battery_ossensor_open(SensorParams *params);
int battery_ossensor_close(void);
char* battery_ossensor_get_vendor(void);

#ifdef __cplusplus
}
#endif
#endif /* _BATTERY_ADC_H */
