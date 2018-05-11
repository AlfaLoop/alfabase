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
#include "frameworks/app_lifecycle.h"
#include "frameworks/app_eventpool.h"
#include "frameworks/ble/ble_api.h"
#include "frameworks/ble/ble_gatt_api.h"
#include "frameworks/ble/ble_uuid_api.h"
#include "libs/util/byteorder.h"

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
// Advertising Data and Scan Response format contains 1 octet for the length.
#define ADV_LEN_FIELD_SIZE                      1UL
//  Advertising Data and Scan Response format contains 1 octet for the AD type
#define ADV_TYPE_FIELD_SIZE                     1UL
#define ADV_DATA_OFFSET                         (ADV_LEN_FIELD_SIZE + ADV_TYPE_FIELD_SIZE)

#define ADV_TYPE_FLAGS_DATA_SIZE                 1UL
#define ADV_TYPE_FLAGS_SIZE                      3UL

// Manufacturer Specific Data.
#define ADV_TYPE_MANUFACTURER_SPECIFIC_DATA      0xFF
//  Size of the Company Identifier Code, which is a part of the Manufacturer Specific Data AD type.
#define ADV_TYPE_MANUF_SPEC_DATA_ID_SIZE         2UL
#define ADV_TYPE_SERV_DATA_16BIT_UUID_SIZE       2UL

/*---------------------------------------------------------------------------*/
static int
adv_builder_append_service16_uuid_data(AdvData *advdata, uint16_t uuid)
{
  uint16_t max_size = ADVERTISE_MAX_DATA_SIZE;
  uint16_t offset = advdata->size;
  uint8_t *p_encoded_data = advdata->data;

  // Validate parameters
  if ( advdata == NULL)
    return EINVAL;

  // Check for buffer overflow.
  if ( (( offset + ADV_DATA_OFFSET) > max_size) ||
       (( offset + ADV_DATA_OFFSET + sizeof(uint16_t)) > max_size))
  {
    return EDSNM;
  }

  // Complete name field in encoded data.
  p_encoded_data[offset] = (uint8_t)(ADV_TYPE_FIELD_SIZE +  sizeof(uint16_t));
  offset                 += ADV_LEN_FIELD_SIZE;
  p_encoded_data[offset] = ADV_TYPE_16BIT_SERVICE_UUID_MORE_AVAILABLE;
  offset                 += ADV_TYPE_FIELD_SIZE;
  memcpy(&p_encoded_data[offset], &uuid, sizeof(uint16_t));
  offset                 += sizeof(uint16_t);

  advdata->size = offset;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
adv_builder_append_service16_data(AdvData *advdata, uint16_t uuid, uint8_t *data, uint8_t data_length)
{
  uint16_t max_size = ADVERTISE_MAX_DATA_SIZE;
  uint16_t offset = advdata->size;
  uint8_t *p_encoded_data = advdata->data;
  uint8_t encoded_size = ADV_TYPE_SERV_DATA_16BIT_UUID_SIZE + data_length;

  // Validate parameters
  if (data == NULL || advdata == NULL)
    return EINVAL;

  // Check for buffer overflow.
  if ( (( offset + ADV_DATA_OFFSET) > max_size) ||
       (( offset + ADV_DATA_OFFSET + encoded_size) > max_size))
  {
    return EDSNM;
  }

  // There is only 1 byte intended to encode length which is (actual_length + ADV_TYPE_FIELD_SIZE)
  if (encoded_size > (0x00FF - ADV_TYPE_FIELD_SIZE))
    return EDSNM;

  // Complete name field in encoded data.
  p_encoded_data[offset] = (uint8_t)(ADV_TYPE_FIELD_SIZE + encoded_size);
  offset                 += ADV_LEN_FIELD_SIZE;
  p_encoded_data[offset] = ADV_TYPE_SERVICE_DATA;
  offset                 += ADV_TYPE_FIELD_SIZE;

  // Encode Company Identifier.
  offset += uint16_encode(uuid, &p_encoded_data[offset]);

  memcpy(&p_encoded_data[offset], data, data_length);
  offset += data_length;

  advdata->size = offset;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
adv_builder_append_complete_local_name_data(AdvData *advdata, uint8_t *name, uint8_t complete_name_length)
{
  uint16_t max_size = ADVERTISE_MAX_DATA_SIZE;
  uint16_t offset = advdata->size;
  uint8_t *p_encoded_data = advdata->data;
  uint16_t rem_adv_data_len;
  uint16_t actual_length;
  uint8_t  adv_data_format;

  // Validate parameters
  if (complete_name_length < 1 || name == NULL || advdata == NULL)
    return EINVAL;

  // Check for buffer overflow.
  if ( (( offset + ADV_DATA_OFFSET) > max_size) ||
       (( offset + ADV_DATA_OFFSET + complete_name_length) > max_size))
  {
    return EDSNM;
  }

  // complete name type
  adv_data_format = ADV_TYPE_COMPLETE_LOCAL_NAME;

  rem_adv_data_len = max_size - offset - ADV_DATA_OFFSET;
  actual_length    = rem_adv_data_len;

  // Short name fits available size.
  if ( complete_name_length <= rem_adv_data_len) {
    actual_length = complete_name_length;
  } else {
    // Else whatever can fit the data buffer will be packed.
    actual_length = rem_adv_data_len;
  }

  // There is only 1 byte intended to encode length which is (actual_length + ADV_TYPE_FIELD_SIZE)
  if (actual_length > (0x00FF - ADV_TYPE_FIELD_SIZE))
    return EDSNM;

  // Complete name field in encoded data.
  p_encoded_data[offset] = (uint8_t)(ADV_TYPE_FIELD_SIZE + actual_length);
  offset                 += ADV_LEN_FIELD_SIZE;
  p_encoded_data[offset] = adv_data_format;
  offset                 += ADV_TYPE_FIELD_SIZE;
  memcpy(&p_encoded_data[offset], name, actual_length);
  offset                 += actual_length;

  advdata->size = offset;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
adv_builder_append_short_local_name_data(AdvData *advdata, uint8_t *name, uint8_t short_name_length)
{
  uint16_t max_size = ADVERTISE_MAX_DATA_SIZE;
  uint16_t offset = advdata->size;
  uint8_t *p_encoded_data = advdata->data;
  uint16_t rem_adv_data_len;
  uint16_t actual_length;
  uint8_t  adv_data_format;

  // Validate parameters
  if (short_name_length < 1 || name == NULL || advdata == NULL)
    return EINVAL;

  // Check for buffer overflow.
  if ( (( offset + ADV_DATA_OFFSET) > max_size) ||
       (( offset + ADV_DATA_OFFSET + short_name_length) > max_size))
  {
    return EDSNM;
  }

  // short name type
  adv_data_format = ADV_TYPE_SHORT_LOCAL_NAME;

  rem_adv_data_len = max_size - offset - ADV_DATA_OFFSET;
  actual_length    = rem_adv_data_len;

  // Short name fits available size.
  if ( short_name_length <= rem_adv_data_len) {
    actual_length = short_name_length;
  } else {
    // Else whatever can fit the data buffer will be packed.
    actual_length = rem_adv_data_len;
  }

  // There is only 1 byte intended to encode length which is (actual_length + ADV_TYPE_FIELD_SIZE)
  if (actual_length > (0x00FF - ADV_TYPE_FIELD_SIZE))
    return EDSNM;

  // Complete name field in encoded data.
  p_encoded_data[offset] = (uint8_t)(ADV_TYPE_FIELD_SIZE + actual_length);
  offset                 += ADV_LEN_FIELD_SIZE;
  p_encoded_data[offset] = adv_data_format;
  offset                 += ADV_TYPE_FIELD_SIZE;
  memcpy(&p_encoded_data[offset], name, actual_length);
  offset                 += actual_length;

  advdata->size = offset;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
adv_builder_append_manufacturer_data(AdvData *advdata, uint16_t manufacturerId, uint8_t *specificData, uint8_t length)
{
  uint32_t data_size = ADV_TYPE_MANUF_SPEC_DATA_ID_SIZE + length;
  uint8_t *p_encoded_data = advdata->data;
  uint16_t offset = advdata->size;
  uint16_t maxsize = ADVERTISE_MAX_DATA_SIZE;

  if (specificData == NULL || advdata == NULL)
   return ENULLP;

  // Check for buffer overflow.
  if ((offset + ADV_DATA_OFFSET + data_size) > maxsize)
   return EDSNM;

  // There is only 1 byte intended to encode length which is (data_size + ADV_TYPE_FIELD_SIZE)
  if (data_size > (0x00FF - ADV_TYPE_FIELD_SIZE))
   return EDSNM;

  // Encode Length and AD Type.
  p_encoded_data[offset]  = (uint8_t)(ADV_TYPE_FIELD_SIZE + data_size);
  offset += ADV_LEN_FIELD_SIZE;
  p_encoded_data[offset]  = ADV_TYPE_MANUFACTURER_SPECIFIC_DATA;
  offset += ADV_TYPE_FIELD_SIZE;

  // Encode Company Identifier.
  offset += uint16_encode(manufacturerId, &p_encoded_data[offset]);

  // Encode additional manufacturer specific data.
  if (length > 0) {
    memcpy(&p_encoded_data[offset], specificData, length);
    offset += length;
  }

  advdata->size = offset;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
adv_builder_init_rawdata(AdvData *advdata)
{
  if (advdata == NULL)
   return ENULLP;

  // reset the adv data
  advdata->size = 0;
  memset(advdata->data, 0x00, ADVERTISE_MAX_DATA_SIZE);

  // Flags must be included in advertising data, and the BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED flag must be set.
  // Encode flags.
  advdata->data[advdata->size]  = (uint8_t)(ADV_TYPE_FIELD_SIZE + ADV_TYPE_FLAGS_DATA_SIZE);
  advdata->size                += ADV_LEN_FIELD_SIZE;
  advdata->data[advdata->size]  = ADV_TYPE_FLAGS;
  advdata->size                += ADV_TYPE_FIELD_SIZE;
  advdata->data[advdata->size]  = BLE_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
  advdata->size                += ADV_TYPE_FLAGS_DATA_SIZE;

  // PRINTF("[adv_builder_init_rawdata] 0x%02x 0x%02x 0x%02x\n", advdata->data[0], advdata->data[1], advdata->data[2]);
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
adv_builder_init_scan_rspdata(AdvData *advdata)
{
  if (advdata == NULL)
   return ENULLP;

  // reset the adv data
  advdata->size = 0;
  memset(advdata->data, 0x00, ADVERTISE_MAX_DATA_SIZE);
  return ENONE;
}
/*---------------------------------------------------------------------------*/
AdvDataBuilder*
CKAdvDataBuilder(void)
{
  static AdvDataBuilder instance;
  instance.addService16bitUUID = adv_builder_append_service16_uuid_data;
  instance.addServiceData = adv_builder_append_service16_data;
  instance.addCompleteLocalName = adv_builder_append_complete_local_name_data;
  instance.addShortLocalName = adv_builder_append_short_local_name_data;
  instance.addManufacturerData = adv_builder_append_manufacturer_data;
  instance.init = adv_builder_init_rawdata;
  instance.initScanRsp = adv_builder_init_scan_rspdata;
  return &instance;
}
/*---------------------------------------------------------------------------*/
