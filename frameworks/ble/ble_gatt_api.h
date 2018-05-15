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
#ifndef _BLE_GATT_API_H
#define _BLE_GATT_API_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "frameworks/ble/ble_uuid_api.h"
#include "frameworks/ble/ble_device_api.h"

#define BLE_GATT_SERVICE_UUID16                             0x1801
#define BLE_GATT_DESCRIPTOR_CFG_UUID16                      0x2902

#define BLE_GATT_SVC_TYPE_END                               0
#define BLE_GATT_SERVICE_TYPE_PRIMARY                       1
#define BLE_GATT_SERVICE_TYPE_SECONDARY                     2

#define BLE_GATT_CHR_PROPS_BROADCAST                        0x0001
#define BLE_GATT_CHR_PROPS_READ                             0x0002
#define BLE_GATT_CHR_PROPS_WRITE_NO_RSP                     0x0004
#define BLE_GATT_CHR_PROPS_WRITE                            0x0008
#define BLE_GATT_CHR_PROPS_NOTIFY                           0x0010
#define BLE_GATT_CHR_PROPS_INDICATE                         0x0020
#define BLE_GATT_CHR_PROPS_AUTH_SIGN_WRITE                  0x0040
#define BLE_GATT_CHR_PROPS_EXTENDED                         0x0080

// Characteristic permission
#define BLE_GATT_CHR_PERMISSION_READ                        0x0001
#define BLE_GATT_CHR_PERMISSION_WRITE                       0x0010

#define BLE_GATT_STATE_UNKNOWN                              0x0000
#define BLE_GATT_STATE_DISCONNECTED                         0x0001
#define BLE_GATT_STATE_CONNECTED                            0x0002

typedef int (* BleGattAccessCallback)(uint16_t conn_handle, uint16_t attr_handle,
                                      void *args);

/*
typedef struct{
  uint8_t *value;
} BleGattDescriptor;*/

typedef struct{
  BleUuid16                       *uuid;
  uint16_t                        *value_handle;
  uint16_t                        *cccd_handle;
  uint16_t                        props;
  uint16_t                        permission;
  uint8_t                         *init_value;
  uint8_t                         init_value_len;
} BleGattCharacteristic;

typedef struct
{
  uint8_t                    type;
  BleUuid128                 *uuid;
  uint16_t                   *handle;
  uint8_t                    characteristic_count;
  BleGattCharacteristic      *characteristics;
} BleGattService;

typedef struct {
  void (*onConnectionStateChanged)(uint16_t conn_handle, uint16_t state);
  void (*onCharacteristicReadRequest)(uint16_t conn_handle, uint16_t handle, uint8_t *value, uint16_t length);
  void (*onCharacteristicWriteRequest)(uint16_t conn_handle, uint16_t handle, uint8_t *value, uint16_t length);
} BleGattServerCallback;

typedef struct {
  void (*onServicesDiscovered)(BleGattCharacteristic  *characteristic);
  void (*onCharacteristicWrite)(BleGattCharacteristic  *characteristic);
  void (*onCharacteristicRead)(BleGattCharacteristic  *characteristic);
  void (*onConnectionStateChanged)(BleDevice *device, int newState);
} BleGattCallback;

/* Back-end */
int ble_gatt_add_service_api(BleGattService *services);
int ble_gatts_notify_characteristic(uint16_t conn_handle, uint16_t handle, uint8_t *value, uint16_t length);
/* Back-end */

#ifdef __cplusplus
}
#endif
#endif /* _BLE_GATT_API_H */
