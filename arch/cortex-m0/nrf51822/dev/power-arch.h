/**
 *  Copyright (c) 2016 AlfaLoop Technology Co., Ltd. All Rights Reserved.
 *
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential.
 *
 *  Attribution — You must give appropriate credit, provide a link to the license, and
 *  indicate if changes were made. You may do so in any reasonable manner, but not in any
 *  way that suggests the licensor endorses you or your use.
 *
 *  NonCommercial — You may not use the material for commercial purposes under unauthorized.
 *
 *  NoDerivatives — If you remix, transform, or build upon the material, you may not
 *  distribute the modified material.
 */
#ifndef _POWER_ARCH_H
#define _POWER_ARCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "dev/power.h"


int power_arch_init(void);
uint8_t power_arch_get_battery_level(void);
uint8_t power_arch_get_charged_status(void);

int adc_arch_channel_init(uint32_t pin, adc_channel_cb_t cb);
int16_t adc_arch_sample(uint32_t pin);
int adc_arch_channel_uninit(uint32_t pin);

#ifdef __cplusplus
}
#endif
#endif /* _POWER_ARCH_H */
