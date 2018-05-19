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
#include "frameworks/hw/hw_api.h"

typedef struct {
	int16_t  heel;
	int16_t  outer_ball;
	int16_t  inner_ball;
	int16_t  thumb;
	uint32_t timestamp;
} ieefp4_data_t;

bool ieefp4_bsp_activated(void);
uint8_t ieefp4_bsp_get_type(void);

int bsp_ieefp4_init(void);
int bsp_ieefp4_open(void *args);
int bsp_ieefp4_write(const void *buf, uint32_t len, uint32_t offset);
int bsp_ieefp4_read(void *buf, uint32_t len, uint32_t offset);
int bsp_ieefp4_subscribe(void *buf, uint32_t len, HWCallbackHandler handler);
int bsp_ieefp4_close(void *args);

PROCESS_NAME(iee_pressure_process);

#ifdef __cplusplus
}
#endif
#endif /* ___IEE_IEEFP4_BSP_SENSOR_H */
