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
#include "nest/nest.h"
#include "nest-driver.h"
#include "nrf.h"
#include "nrf_soc.h"
#include "nrf_sdm.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_types.h"
#include "nordic_common.h"
#include "softdevice_handler.h"
/*---------------------------------------------------------------------------*/
#if defined(DEBUG_ENABLE)
#define DEBUG_MODULE 1
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
#define POOL_SIZE NEST_ADD_APP_GATTS_SERVICE

typedef struct {
  bool used;
  uint8_t service_type;
  uint16_t service_handle;
  uint16_t service_uuid;
  uint8_t vs_uuid[16];
} nest_service_instance_t;

static nest_service_instance_t pool[POOL_SIZE];
static int ble_gatt_num_services = 0;

/*---------------------------------------------------------------------------*/
/* Each service requires: o 1 service, o 1 attribute */
int
nrf_gatts_add_service(nest_bleservice_t *p_service)
{
  uint32_t        err_code;
  ble_uuid_t      service_uuid;
  ble_uuid128_t   vendor_srv_uuid;
  int idx;
  bool registed = false;

  if (p_service == NULL) {
    return ENULLP;
  }

  if (ble_gatt_num_services >= POOL_SIZE) {
    return ENOMEM;
  }

  // Add a vendor specfit 128-bit UUID
  if(p_service->bleuuid.type == NEST_BLE_UUID_TYPE_VENDOR) {

    PRINTF("[nest driver gatts addserivce] add vendor uuid\n");
    // get hte uuid from pool
    registed = false;
    for (idx = 0; idx < POOL_SIZE; idx++) {
      if ( (pool[idx].used) )
      {
        PRINTF("[nest driver gatts addserivce] idx %d uuid: ", idx);
        for (int i = 0; i < 16; i++)
          PRINTF("%2X ", pool[idx].vs_uuid[i]);
        PRINTF("\n");

        PRINTF("[nest driver gatts addserivce] service uuid: ");
        for (int i = 0; i < 16; i++)
          PRINTF("%2X ", p_service->vendor_uuid.uuid128[i]);
        PRINTF("\n");

        if ( ( memcmp(&pool[idx].vs_uuid[0], &p_service->vendor_uuid.uuid128[0], 16) == 0)) {
          PRINTF("[nest driver gatts addserivce] uuid match, registered\n");
          registed = true;
          break;
        } else {
          PRINTF("[nest driver gatts addserivce] uuid not match, add it\n");
        }
      }
    }

    if (registed) {
      // service_uuid.type = BLE_UUID_TYPE_VENDOR_BEGIN;
      // p_service->service_type = pool[idx].service_handle;
      p_service->handle = pool[idx].service_handle;
      p_service->bleuuid.type = pool[idx].service_type;
      service_uuid.type = pool[idx].service_type;
      // p_service->bleuuid.type = service_uuid.type;
      PRINTF("[nest driver gatts addserivce] vendor service already registed type %d\n", pool[idx].service_type);
    } else {
      memcpy(vendor_srv_uuid.uuid128, p_service->vendor_uuid.uuid128, sizeof(nest_bleuuid128_t));
      PRINTF("[nest driver gatts addserivce] sd ble uuid vs add\n");
      err_code = sd_ble_uuid_vs_add(&vendor_srv_uuid, &(service_uuid.type));
      if (err_code != NRF_SUCCESS){
        PRINTF("[nest driver gatts addserivce] nrf add service error %d\n", err_code);
        if (err_code == NRF_ERROR_INVALID_ADDR) {
          return EFAULT;
        } else if (err_code == NRF_ERROR_NO_MEM) {
          return ENOMEM;
        } else {
          return EINTERNAL;
        }
      }

      p_service->bleuuid.type = service_uuid.type;
      service_uuid.uuid = p_service->bleuuid.uuid;
      PRINTF("[nest driver gatts addserivce] sd ble gatts service add\n");
      err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                          &(service_uuid),
                                          &(p_service->handle));
      if (err_code != NRF_SUCCESS){
        PRINTF("[nest driver gatts addserivce] add error %d \n", err_code);
        if (err_code == NRF_ERROR_INVALID_ADDR) {
          return EFAULT;
        } else if (err_code == NRF_ERROR_NO_MEM) {
          return ENOMEM;
        } else if (err_code == NRF_ERROR_INVALID_PARAM) {
          return EINVAL;
        } else if (err_code == NRF_ERROR_FORBIDDEN) {
          return EPERM;
        } else {
          return EINTERNAL;
        }
      }

      // update the pool list
      PRINTF("[nest driver gatts addserivce] update the pool list\n");
      for (idx = 0; idx < POOL_SIZE; idx++) {
        if (!pool[idx].used ) {
          pool[idx].used = true;
          pool[idx].service_handle = p_service->handle;
          pool[idx].service_uuid = service_uuid.uuid;
          memcpy(&pool[idx].vs_uuid[0], &p_service->vendor_uuid.uuid128[0], 16);
          ble_gatt_num_services++;
          break;
        }
      }
    }
  }
  else if (p_service->bleuuid.type == NEST_BLE_UUID_TYPE_BLE) {
    service_uuid.type = BLE_UUID_TYPE_BLE;
    service_uuid.uuid = p_service->bleuuid.uuid;

    registed = false;
    for (idx = 0; idx < POOL_SIZE; idx++) {
      if ( (pool[idx].used) &&
         (pool[idx].service_uuid == p_service->bleuuid.uuid) )
      {
        PRINTF("[nest driver gatts addserivce] service 16 bits uuid already registed 0x%4x\n", pool[idx].service_uuid );
        registed = true;
        break;
      }
    }

    if (registed) {
      p_service->handle = pool[idx].service_handle;
      PRINTF("[nest driver gatts addserivce] service uuid already registed\n");
    } else {
      err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                          &(service_uuid),
                                          &(p_service->handle));
      if (err_code != NRF_SUCCESS){
        PRINTF("[nest driver gatts addserivce] add error %d \n", err_code);
        if (err_code == NRF_ERROR_INVALID_ADDR) {
          return EFAULT;
        } else if (err_code == NRF_ERROR_NO_MEM) {
          return ENOMEM;
        } else if (err_code == NRF_ERROR_INVALID_PARAM) {
          return EINVAL;
        } else if (err_code == NRF_ERROR_FORBIDDEN) {
          return EPERM;
        } else {
          return EINTERNAL;
        }
      }

      // update the pool list
      for (idx = 0; idx < POOL_SIZE; idx++) {
        if (!pool[idx].used ) {
          pool[idx].used = true;
          pool[idx].service_handle = p_service->handle;
          pool[idx].service_uuid = service_uuid.uuid;
          break;
        }
      }
    }
  }
  else {
    return EINVAL;
  }
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
nrf_gatts_init_service_pool(void)
{
  PRINTF("[nest driver gatts addserivce] reset the service pool\n");
  ble_gatt_num_services = 0;
  for (int i = 0; i < POOL_SIZE; i++) {
    pool[i].used = false;
    pool[i].service_handle = 0;
    pool[i].service_uuid = 0;
    pool[i].service_type = 0;
    memset(&pool[i].vs_uuid[0], 0x00, 16);
  }
}
/*---------------------------------------------------------------------------*/
