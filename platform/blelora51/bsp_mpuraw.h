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
#ifndef ___BSP_MPURAW_SENSOR_H
#define ___BSP_MPURAW_SENSOR_H
#ifdef __cplusplus
extern "C" {
#endif

#include "contiki.h"
#include "frameworks/hw/hw_api.h"

typedef struct {
	int16_t accel[3];
	int16_t gyro[3];
	int16_t compass[3];
	uint32_t timestamp;
} motion_data_event_t;

int bsp_mpu9250_raw_init(void);
int bsp_mpu9250_raw_open(void *args);
int bsp_mpu9250_raw_write(const void *buf, uint32_t len, uint32_t offset);
int bsp_mpu9250_raw_read(void *buf, uint32_t len, uint32_t offset);
int bsp_mpu9250_raw_subscribe(void *buf, uint32_t len, HWCallbackHandler handler);
int bsp_mpu9250_raw_close(void *args);

void mpu9250_raw_data_update(int16_t *accel, int16_t *gyro, int16_t *compass, uint32_t timestamp);


#ifdef __cplusplus
}
#endif
#endif /* ___BSP_MPURAW_SENSOR_H */
