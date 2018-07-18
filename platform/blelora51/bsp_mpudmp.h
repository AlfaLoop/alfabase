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

typedef struct {
  uint8_t type;
} motion_data_event_t;

int bsp_mpu9250_dmp_init(void);
int bsp_mpu9250_dmp_open(void *args);
int bsp_mpu9250_dmp_write(const void *buf, uint32_t len, uint32_t offset);
int bsp_mpu9250_dmp_read(void *buf, uint32_t len, uint32_t offset);
int bsp_mpu9250_dmp_subscribe(void *buf, uint32_t len, HWCallbackHandler handler);
int bsp_mpu9250_dmp_close(void *args);

void mpu9250_dmp_data_update(uint8_t source);


#ifdef __cplusplus
}
#endif
#endif /* ___BSP_MPUDMP_SENSOR_H */
