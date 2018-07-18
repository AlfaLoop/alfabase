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
#include "contiki.h"
#include "loader/symtab.h"
#include "frameworks/app_lifecycle.h"
#include "frameworks/app_eventpool.h"
#include "frameworks/ble/ble_api.h"
#include "frameworks/ble/ble_gatt_api.h"
#include "frameworks/ble/ble_uuid_api.h"
/*---------------------------------------------------------------------------*/
#if defined(DEBUG_ENABLE)
#define DEBUG_MODULE 0
#if DEBUG_MODULE
#include "dev/syslog.h"
#define PRINTF(...) syslog(__VA_ARGS__)
#else
#define PRINTF(...)
#endif  /* DEBUG_MODULE */
#else
#define PRINTF(...)
#endif  /* DEBUG_ENABLE */

/*---------------------------------------------------------------------------*/
int
ble_gatt_add_service_api(BleGattService *service)
{
  uint32_t errcode = ENONE;
  uint8_t total_characteristic;
  uint16_t uuid16;
  nest_bleservice_t nest_bleservice;
  nest_blecharacteristic_t nest_characteristic;
  // PRINTF("[ble_gatt_api] NEST.gatts_add_service\n");

  if (service == NULL) {
    return ENULLP;
  }

  if (service->characteristic_count > NEST_ADD_APP_GATTS_CHARACTERISTIC) {
    return EINVAL;
  }

  // copy the 128 bits UUID
  nest_bleservice.bleuuid.type = NEST_BLE_UUID_TYPE_VENDOR;
  memcpy(&nest_bleservice.vendor_uuid.uuid128[0], &service->uuid->value[0], 16);

  // copy the 16 bits UUID
  uuid16 = (uint16_t) (nest_bleservice.vendor_uuid.uuid128[13] << 8) | nest_bleservice.vendor_uuid.uuid128[12];
  nest_bleservice.bleuuid.uuid = uuid16;

  // call low level layer to add service
  errcode = NEST.gatts_add_service(&nest_bleservice);
  if (errcode != ENONE) {
    PRINTF("[ble_gatt_api] NEST.gatts_add_service error %d\n", errcode);
    return errcode;
  }

  // retrive the service handle
  *service->handle = nest_bleservice.handle;
  // PRINTF("[ble_gatt_api] service handle %d %d\n", nest_bleservice.handle, *service->handle);

  // start to add charateristic
  total_characteristic = service->characteristic_count;
  // PRINTF("[ble_gatt_api] total_characteristic %d \n", total_characteristic);
  for (uint8_t i = 0; i < total_characteristic; i++) {
    BleGattCharacteristic *bleGattChar = &service->characteristics[i];

    // setup the characteristic properties
    memset(&nest_characteristic, 0x00, sizeof(nest_blecharacteristic_t));
    nest_characteristic.uuid = bleGattChar->uuid->value;
    if (bleGattChar->props & BLE_GATT_CHR_PROPS_NOTIFY)
      nest_characteristic.props.notify = 1;
    if (bleGattChar->props & BLE_GATT_CHR_PROPS_INDICATE)
      nest_characteristic.props.indicate = 1;
    if (bleGattChar->props & BLE_GATT_CHR_PROPS_BROADCAST)
      nest_characteristic.props.broadcast = 1;
    if (bleGattChar->props & BLE_GATT_CHR_PROPS_READ)
      nest_characteristic.props.read = 1;
    if (bleGattChar->props & BLE_GATT_CHR_PROPS_WRITE_NO_RSP)
      nest_characteristic.props.write_wo_resp = 1;
    if (bleGattChar->props & BLE_GATT_CHR_PROPS_WRITE)
      nest_characteristic.props.write = 1;
    if (bleGattChar->props & BLE_GATT_CHR_PROPS_AUTH_SIGN_WRITE)
      nest_characteristic.props.auth_signed_wr = 1;

    // it has a CCCD if the characteristic permits notifications or indications.
    if (bleGattChar->permission & BLE_GATT_CHR_PERMISSION_READ)
      nest_characteristic.permission.read = 1;
    if (bleGattChar->permission & BLE_GATT_CHR_PERMISSION_WRITE)
      nest_characteristic.permission.write = 1;


    if (bleGattChar->init_value != NULL) {
      nest_characteristic.init_value = bleGattChar->init_value;
      nest_characteristic.init_value_len = bleGattChar->init_value_len;
    }

    // PRINTF("[ble_gatt_api] add characteristic\n");
    errcode = NEST.gatts_add_characteristic(&nest_bleservice, &nest_characteristic);
    if (errcode != ENONE) {
      PRINTF("[ble_gatt_api] NEST.gatts_add_characteristic error %d\n", errcode);
      return errcode;
    }

    // PRINTF("[ble_gatt_api] value_handle %d cccd_handle %d\n", nest_characteristic.value_handle, nest_characteristic.cccd_handle);

    *bleGattChar->value_handle = nest_characteristic.value_handle;
    *bleGattChar->cccd_handle = nest_characteristic.cccd_handle;
  }
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
ble_gatts_notify_characteristic(uint16_t conn_handle, uint16_t handle, uint8_t *value, uint16_t length)
{
  int errcode = ENONE;
  errcode = NEST.gatts_handle_value(conn_handle, NEST_BLE_HVX_TYPE_NOTIFICATION, handle, value, length);
  if (errcode != ENONE) {
    PRINTF("[ble_gatt_api] NEST handle notify error %d\n", errcode);
  }
  return errcode;
}
/*---------------------------------------------------------------------------*/
