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
#ifndef _BLE_SCAN_API_H
#define _BLE_SCAN_API_H
#ifdef __cplusplus
extern "C" {
#endif

#include "frameworks/ble/ble_types_api.h"
#include "nest/nest.h"

/* Framework API */
typedef enum {
  SCAN_INTERVAL_LEVEL_0 = 50,            // 50ms
  SCAN_INTERVAL_LEVEL_1 = 100,           // 100ms
  SCAN_INTERVAL_LEVEL_2 = 200,           // 200ms
  SCAN_INTERVAL_LEVEL_3 = 300,           // 300ms
  SCAN_INTERVAL_LEVEL_4 = 400,           // 400ms
  SCAN_INTERVAL_LEVEL_5 = 500,           // 500ms
  SCAN_INTERVAL_LEVEL_6 = 650,           // 650ms
  SCAN_INTERVAL_LEVEL_7 = 750,           // 750ms
  SCAN_INTERVAL_LEVEL_8 = 820            // 820ms
} ScanIntervalLevel;

typedef struct{
  BleDevice    device;
  int8_t       rssi;                  			// Received Signal Strength Indication in dBm.
  uint8_t		   scan_response:1;
  uint8_t 	   type:2;
  uint8_t      len:5;
  uint8_t      data[ADVERTISE_MAX_DATA_SIZE];    // Advertising or scan response data.
} ScanRecord;

typedef struct {
  void (*onScanCompleted)(void);
  void (*onScanAbort)(void);
  void (*onScanResult)(ScanRecord *record);
} BleScanCallback;
/* Framework API */

/* Back-end */

/* Back-end */

#ifdef __cplusplus
}
#endif
#endif /* _BLE_SCAN_API_H */
