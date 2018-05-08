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
#define POOL_SIZE  20

typedef struct {
  bool used;
  uint16_t char_uuid;
  uint16_t service_handle;
  uint16_t value_handle;
  uint16_t cccd_handle;
} nest_characteristic_instance_t;

static nest_characteristic_instance_t pool[POOL_SIZE];
/*---------------------------------------------------------------------------*/
/* Each characteristic requires: o 1 characteristic, o 2 attributes*/
int
nrf_gatts_add_characteristic(nest_bleservice_t *p_service,
                            nest_blecharacteristic_t *p_characteristics)
{
  uint32_t err_code = ENONE;
  ble_gatts_char_md_t       char_md;
  ble_gatts_attr_md_t       cccd_md;
  ble_gatts_attr_t          attr_char_value;
  ble_uuid_t                char_uuid;
  ble_gatts_attr_md_t       attr_md;
  ble_gatts_char_handles_t  char_handles;
  uint16_t service_handle = p_service->handle;
  int idx;
  bool registed = false;

  // This CCCD descriptor will be added automatically for any characteristic that
  // has either the Notify or the Indicate properties.
  if (p_characteristics->props.notify || p_characteristics->props.indicate) {
    // set cccd
    memset(&cccd_md, 0, sizeof(cccd_md));
    if (p_characteristics->permission.read)
      BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    if (p_characteristics->permission.write)
      BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;
  }

  memset(&char_md, 0, sizeof(char_md));
  if (p_characteristics->props.notify)
    char_md.char_props.notify = 1;
  if (p_characteristics->props.broadcast)
    char_md.char_props.broadcast = 1;
  if (p_characteristics->props.read)
    char_md.char_props.read = 1;
  if (p_characteristics->props.write_wo_resp)
    char_md.char_props.write_wo_resp = 1;
  if (p_characteristics->props.write)
    char_md.char_props.write = 1;
  if (p_characteristics->props.notify)
    char_md.char_props.notify = 1;
  if (p_characteristics->props.indicate)
    char_md.char_props.indicate = 1;
  if (p_characteristics->props.auth_signed_wr)
    char_md.char_props.auth_signed_wr = 1;

  char_md.p_char_user_desc  = NULL;
  char_md.p_char_pf         = NULL;
  char_md.p_user_desc_md    = NULL;
  if (p_characteristics->props.notify || p_characteristics->props.indicate)
    char_md.p_cccd_md         = &cccd_md;
  else
    char_md.p_cccd_md         = NULL;
  char_md.p_sccd_md         = NULL;

  // set char_uuid
  char_uuid.type = p_service->bleuuid.type;
  char_uuid.uuid = p_characteristics->uuid;

  // set attr_md
  memset(&attr_md, 0, sizeof(attr_md));
  if (p_characteristics->permission.read)
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
  if (p_characteristics->permission.write)
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
  attr_md.vloc    = BLE_GATTS_VLOC_STACK;
  attr_md.rd_auth = 0;
  attr_md.wr_auth = 0;
  attr_md.vlen    = 1;

  memset(&attr_char_value, 0, sizeof(attr_char_value));
  attr_char_value.p_uuid    = &char_uuid;
  attr_char_value.p_attr_md = &attr_md;
  attr_char_value.init_len  = p_characteristics->init_value_len;
  attr_char_value.init_offs = 0;
  attr_char_value.max_len   = BLE_NEST_FIX_DATA_LEN;
  if (p_characteristics->init_value != NULL)
    attr_char_value.p_value = p_characteristics->init_value;

  for (idx = 0; idx < POOL_SIZE; idx++) {
    if ( (pool[idx].used) &&
         (pool[idx].char_uuid == p_characteristics->uuid) &&
         (pool[idx].service_handle == service_handle) )
    {
      registed = true;
      break;
    }
  }

  if (registed) {
    PRINTF("[nrf add characteristic] registed\n");
    p_characteristics->value_handle = pool[idx].value_handle;
    p_characteristics->cccd_handle = pool[idx].cccd_handle;
    err_code = ENONE;
  } else {
    err_code = sd_ble_gatts_characteristic_add(service_handle,
                                             &char_md,
                                             &attr_char_value,
                                             &char_handles);
    if (err_code != NRF_SUCCESS){
      PRINTF("[nrf add characteristic] error %d\n", err_code);
      if (err_code == NRF_ERROR_INVALID_ADDR) {
        return EFAULT;
      } else if (err_code == NRF_ERROR_NO_MEM) {
        return ENOMEM;
      } else if (err_code == NRF_ERROR_INVALID_PARAM) {
        return EINVAL;
      } else if (err_code == NRF_ERROR_INVALID_STATE) {
        return EINVALSTATE;
      } else if (err_code == NRF_ERROR_FORBIDDEN) {
        return EPERM;
      } else if (err_code == NRF_ERROR_DATA_SIZE) {
        return ENOSUPPORT;
      } else {
        return EINTERNAL;
      }
    } else {
      for (idx = 0; idx < POOL_SIZE; idx++) {
        if (!pool[idx].used )
        {
          pool[idx].used = true;
          pool[idx].char_uuid = p_characteristics->uuid;
          pool[idx].service_handle = service_handle;
          pool[idx].value_handle = char_handles.value_handle;
          pool[idx].cccd_handle = char_handles.cccd_handle;
          break;
        }
      }
    }
    p_characteristics->value_handle = char_handles.value_handle;
    p_characteristics->cccd_handle = char_handles.cccd_handle;
  }
  return err_code;
}
/*---------------------------------------------------------------------------*/
int
nrf_gatts_init_characteristic_pool(void)
{
  for (int i = 0; i < POOL_SIZE; i++) {
    pool[i].used = false;
    pool[i].char_uuid = 0;
    pool[i].service_handle = 0;
    pool[i].value_handle = 0;
    pool[i].cccd_handle = 0;
  }
}
/*---------------------------------------------------------------------------*/
