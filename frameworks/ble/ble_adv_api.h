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
#ifndef _BLE_ADV_API_H
#define _BLE_ADV_API_H
#ifdef __cplusplus
extern "C" {
#endif

#include "frameworks/ble/ble_types_api.h"

/* Framework API */
typedef enum {
  ADV_INTERVAL_LEVEL_0 = 152,           // 152.5ms
  ADV_INTERVAL_LEVEL_1 = 318,           // 318.75ms
  ADV_INTERVAL_LEVEL_2 = 546,           // 546.25ms
  ADV_INTERVAL_LEVEL_3 = 852,           // 852.5ms
  ADV_INTERVAL_LEVEL_4 = 1022,          // 1022.5ms
  ADV_INTERVAL_LEVEL_5 = 2045,          // 2045.0ms
  ADV_INTERVAL_LEVEL_6 = 4082,          // 4082.5ms
  ADV_INTERVAL_LEVEL_7 = 5120,          // 5120.0ms
  ADV_INTERVAL_LEVEL_8 = 10040          // 10040.0ms
} AdvIntervalLevel;

typedef struct {
  void (*onSendout)(void);
} BleAdvertiseCallback;

typedef struct {
  uint16_t  size;
  uint8_t   data[31];
} AdvData;

typedef struct {
  int (*addService16bitUUID)(AdvData *advdata, uint16_t uuid);
  int (*addServiceData)(AdvData *advdata, uint16_t uuid, uint8_t *data, uint8_t data_length);
  int (*addCompleteLocalName)(AdvData *advdata, uint8_t *name, uint8_t complete_name_length);
  int (*addShortLocalName)(AdvData *advdata, uint8_t *name, uint8_t short_name_length);
  int (*addManufacturerData)(AdvData *advdata, uint16_t manufacturerId, uint8_t *specificData, uint8_t length);
  int (*init)(AdvData *advdata);
  int (*initScanRsp)(AdvData *advdata);
} AdvDataBuilder;

AdvDataBuilder* CKAdvDataBuilder(void);
/* Framework API */


/* Back-end */
/* Back-end */

#ifdef __cplusplus
}
#endif
#endif /* _BLE_ADV_API_H */
