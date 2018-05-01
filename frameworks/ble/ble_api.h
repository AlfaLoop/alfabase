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
#ifndef _BLE_API_H
#define _BLE_API_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "frameworks/ble/ble_uuid_api.h"
#include "frameworks/ble/ble_device_api.h"
#include "frameworks/ble/ble_gatt_api.h"
#include "frameworks/ble/ble_scan_api.h"
#include "frameworks/ble/ble_adv_api.h"
#include "frameworks/ble/ble_types_api.h"
#include "nest/nest.h"

/* Framework API */
typedef struct{
  int (*getMacAddress)(uint8_t *address);
  int (*setTxPower)(int8_t power);
  int (*setDeviceName)(const char *name, int len);
  int (*setAdvertisementData)(AdvData *advdata, AdvData *scanrsp_data);
  int (*startAdvertising)(AdvIntervalLevel level, const BleAdvertiseCallback *advCallback, const BleGattServerCallback *gattServerCallback);
  int (*stopAdvertising)(void);
  int (*startScan)(ScanIntervalLevel level, const BleScanCallback *callback);
  int (*stopScan)(void);
  int (*addService)(BleGattService *service);
  int (*notifyCharacteristic)(uint16_t conn_handle, uint16_t handle, uint8_t *value, uint16_t length);
} BleManager;

BleManager* CKBleManager(void);
/* Framework API */

/* Back-end */
#define BLE_CHARACTERISTIC_WRITE_REQUEST    0x00
#define BLE_CHARACTERISTIC_READ_REQUEST     0x01

typedef struct {
  uint8_t  type;
  uint16_t conn_handle;
  uint16_t handle;
  uint8_t  value[20];
  uint16_t length;
} BleCharacteristicEvent;

void ble_api_init(void);

// ble scan api
bool ble_scan_api_attached(void);
uint16_t ble_scan_api_interval_level(void);
void ble_scan_completed(uint8_t type);


// ble advertising api
bool ble_adv_api_attached(void);
void ble_adv_api_sendout(void);
void ble_adv_api_params_retrieve(nest_broadcast_params_t *params);

// ble gatts
void ble_gatts_write_event_handler(uint16_t conn_handle, uint16_t handle,
                                                uint8_t *value, uint16_t length);
void ble_api_connection_event_handler(uint16_t conn_handle, uint16_t state);

int8_t ble_api_get_tx_power(void);
/* Back-end */


#ifdef __cplusplus
}
#endif
#endif /* _BLE_API_H */
