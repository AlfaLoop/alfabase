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
#define POOL_SIZE  4

typedef struct {
  bool used;
  uint16_t service_handle;
  uint16_t service_uuid;
  uint8_t vs_uuid[16];
} nest_service_instance_t;

static nest_service_instance_t pool[POOL_SIZE];
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

  // Add a vendor specfit 128-bit UUID
  if(p_service->bleuuid.type == NEST_BLE_UUID_TYPE_VENDOR) {
    registed = false;
    for (idx = 0; idx < POOL_SIZE; idx++) {
      if ( (pool[idx].used) &&
           ( memcmp(&pool[idx].vs_uuid[0], &p_service->vendor_uuid.uuid128[0], 16) == 0) )
      {
        registed = true;
        break;
      }
    }
    if (registed) {
      service_uuid.type = BLE_UUID_TYPE_VENDOR_BEGIN;
    } else {
      memcpy(vendor_srv_uuid.uuid128, p_service->vendor_uuid.uuid128, sizeof(nest_bleuuid128_t));
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
    }

  } else if (p_service->bleuuid.type == NEST_BLE_UUID_TYPE_BLE) {
    service_uuid.type = BLE_UUID_TYPE_BLE;
  } else {
    return EINVAL;
  }

  service_uuid.uuid = p_service->bleuuid.uuid;
  p_service->bleuuid.type = service_uuid.type;

  // Add a gatts service
  registed = false;
  for (idx = 0; idx < POOL_SIZE; idx++) {
    if ( (pool[idx].used) &&
         (pool[idx].service_uuid == p_service->bleuuid.uuid) )
    {
      registed = true;
      break;
    }
  }
  if (registed) {
    p_service->handle = pool[idx].service_handle;
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
    } else {
      for (idx = 0; idx < POOL_SIZE; idx++) {
        if (!pool[idx].used )
        {
          pool[idx].used = true;
          pool[idx].service_handle = p_service->handle;
          pool[idx].service_uuid = service_uuid.uuid;
          if (service_uuid.uuid == BLE_UUID_TYPE_VENDOR_BEGIN) {
            memcpy(&pool[idx].vs_uuid[0], p_service->vendor_uuid.uuid128[0], 16);
          }
          break;
        }
      }
    }
  }
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
nrf_gatts_init_service_pool(void)
{
  for (int i = 0; i < POOL_SIZE; i++) {
    pool[i].used = false;
    pool[i].service_handle = 0;
    pool[i].service_uuid = 0;
    memset(&pool[i].vs_uuid[0], 0x00, 16);
  }
}
/*---------------------------------------------------------------------------*/
