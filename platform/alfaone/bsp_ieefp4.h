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
#ifndef ___IEE_IEEFP4_BSP_SENSOR_H
#define ___IEE_IEEFP4_BSP_SENSOR_H
#ifdef __cplusplus
extern "C" {
#endif

#include "contiki.h"

int ieefp4_bsp_init(void);
int ieefp4_bsp_poweron(void);
int ieefp4_bsp_poweroff(void);
bool ieefp4_bsp_activated(void);
uint8_t ieefp4_bsp_get_type(void);
int16_t ieefp4_bsp_get_thumb(void);
int16_t ieefp4_bsp_get_outer_ball(void);
int16_t ieefp4_bsp_get_inner_ball(void);
int16_t ieefp4_bsp_get_heel(void);

int hw_bsp_iee_footpressure4_open(void *args);
int hw_bsp_iee_footpressure4_write(const void *buf, uint32_t len, uint32_t *offset);
int hw_bsp_iee_footpressure4_read(void *buf, uint32_t len, uint32_t offset);
int hw_bsp_iee_footpressure4_subscribe(void *buf, uint32_t len, HWCallbackHandler handler);
int hw_bsp_iee_footpressure4_close(void *args);

PROCESS_NAME(iee_pressure_process);

#ifdef __cplusplus
}
#endif
#endif /* ___IEE_IEEFP4_BSP_SENSOR_H */
