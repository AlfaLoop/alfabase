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
#ifndef ___BSP_MPUDMP_SENSOR_H
#define ___BSP_MPUDMP_SENSOR_H
#ifdef __cplusplus
extern "C" {
#endif

#include "contiki.h"
#include "frameworks/hw/hw_api.h"

typedef struct {
	float value[3];
	int32_t data[3];
	uint32_t timestamp;
} mems_data_t;

typedef struct {
	float value[4];
	int32_t data[4];
	uint32_t timestamp;
} quat_data_t;

typedef struct {
	float value[3];
	uint32_t timestamp;
} linear_accel_data_t;

typedef struct {
	float value[3];
	uint32_t timestamp;
} gravity_vector_t;

typedef struct {
	float value;
	int32_t data;
	uint32_t timestamp;
} heading_data_t;

int bsp_mpu9250_dmp_init(void);
int bsp_mpu9250_dmp_open(void *args);
int bsp_mpu9250_dmp_write(const void *buf, uint32_t len, uint32_t offset);
int bsp_mpu9250_dmp_read(void *buf, uint32_t len, uint32_t offset);
int bsp_mpu9250_dmp_subscribe(void *buf, uint32_t len, HWCallbackHandler handler);
int bsp_mpu9250_dmp_close(void *args);

#ifdef __cplusplus
}
#endif
#endif /* ___BSP_MPUDMP_SENSOR_H */
