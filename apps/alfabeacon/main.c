/*
* Copyright (C) 2018 AlfaLoop Technology Co., Ltd.
*
* Licensed under the Apache License, Version 2.0 (the "License";
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* 	  www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#include "alfabase.h"
#include "ibeacon_profile.h"
#include "radio_profile.h"

#if defined(BOARD_ALFAAA)
#elif defined(BOARD_ALFA2477)
#elif defined(BOARD_ALFAUSB)
#elif defined(BOARD_ALFA2477S)
#include "alfa2477s_profile.h"
#else
#error need define the BOARD_TYPE in cflag setting (app.conf)
#endif
/*---------------------------------------------------------------------------*/
const static uint32_t FILE_KEY_IBEACON_UUID = 0x00000001;
const static uint32_t FILE_KEY_IBEACON_MAJOR = 0x00000002;
const static uint32_t FILE_KEY_IBEACON_MINOR = 0x00000003;
const static uint32_t FILE_KEY_IBEACON_TXM = 0x00000004;
const static uint32_t FILE_KEY_RADIO_INTERVAL = 0x00000005;
const static uint32_t FILE_KEY_RADIO_TXPOWER = 0x00000006;
#if defined(BOARD_ALFA2477S)
const static uint32_t FILE_KEY_2477S_RF_ATTE = 0x00000007;
#endif
/*---------------------------------------------------------------------------*/
const static char *device_name = "AlfaBeacon";
static uint8_t m_mac_address[6];
static bool is_connected = false;
static uint16_t peripheral_conn_handle;
static uint8_t service_data_packetes[8];
static int adv_sendout_counter = 0;
/*---------------------------------------------------------------------------*/
/* Framework API instance */
static Logger *logger;
static Process *process;
static AdvDataBuilder *adv_builder;
static BleManager *ble_manager;
static Device *device;
static AdvData advdata;
static AdvData scan_rsp_advdata;
static FileIO *fileio;
/*---------------------------------------------------------------------------*/
#if defined(BOARD_ALFA2477S)
static HWDriver *hwdriver_rfatte;
static HWDriver *hwdriver_led;
static HWDriver *hwdriver_button;
static HWDriver *hwdriver_buzzer;
#endif
/*---------------------------------------------------------------------------*/
/* ble ibeacon service handle */
static uint16_t ble_srv_ibeacon_handle;
static uint16_t ble_attr_ibeacon_uuid_handle;
static uint16_t ble_attr_ibeacon_major_handle;
static uint16_t ble_attr_ibeacon_minor_handle;
static uint16_t ble_attr_ibeacon_txm_handle;
static uint8_t
ble_adv_manu_ibeacon_data[DEFAULT_IBEACON_BEACON_INFO_LENGTH] =                    /**< Information advertised by the Beacon. */
{
  DEFAULT_IBEACON_DEVICE_TYPE,     // Manufacturer specific information. Specifies the device type in this
                       // implementation.
  DEFAULT_IBEACON_ADV_DATA_LENGTH, // Manufacturer specific information. Specifies the length of the
                       // manufacturer specific data in this implementation.
  DEFAULT_IBEACON_UUID,     // 128 bit UUID value.
  DEFAULT_IBEACON_MAJOR_VALUE,     // Major arbitrary value that can be used to distinguish between Beacons.
  DEFAULT_IBEACON_MINOR_VALUE,     // Minor arbitrary value that can be used to distinguish between Beacons.
  DEFAULT_IBEACON_MEASURED_RSSI    // Manufacturer specific information. The Beacon's measured TX power in this implementation.
};
/*---------------------------------------------------------------------------*/
static BleGattService g_ble_gatt_alfa_ibeacon_service = {
  .type = BLE_GATT_SERVICE_TYPE_PRIMARY,
  .uuid = &gatt_alfa_ibeacon_service_uuid,
  .handle = &ble_srv_ibeacon_handle,
  .characteristic_count = 4,
  .characteristics = (BleGattCharacteristic[]) { {
    .uuid = &gatt_alfa_ibeacon_chr_uuid,
    .value_handle = &ble_attr_ibeacon_uuid_handle,
    .props = BLE_GATT_CHR_PROPS_READ | BLE_GATT_CHR_PROPS_WRITE | BLE_GATT_CHR_PROPS_WRITE_NO_RSP,
    .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
    .init_value = &ble_adv_manu_ibeacon_data[2],
    .init_value_len = 16,
  }, {
    .uuid = &gatt_alfa_ibeacon_chr_major,
    .value_handle = &ble_attr_ibeacon_major_handle,
    .props = BLE_GATT_CHR_PROPS_READ | BLE_GATT_CHR_PROPS_WRITE | BLE_GATT_CHR_PROPS_WRITE_NO_RSP,
    .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
    .init_value = &ble_adv_manu_ibeacon_data[18],
    .init_value_len = 2,
  }, {
    .uuid = &gatt_alfa_ibeacon_chr_minor,
    .value_handle = &ble_attr_ibeacon_minor_handle,
    .props = BLE_GATT_CHR_PROPS_READ | BLE_GATT_CHR_PROPS_WRITE | BLE_GATT_CHR_PROPS_WRITE_NO_RSP,
    .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
    .init_value = &ble_adv_manu_ibeacon_data[20],
    .init_value_len = 2,
  }, {
    .uuid = &gatt_alfa_ibeacon_chr_txm,
    .value_handle = &ble_attr_ibeacon_txm_handle,
    .props = BLE_GATT_CHR_PROPS_READ | BLE_GATT_CHR_PROPS_WRITE | BLE_GATT_CHR_PROPS_WRITE_NO_RSP,
    .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
    .init_value = &ble_adv_manu_ibeacon_data[22],
    .init_value_len = 1,
  }, {
    0, /* End: No more characteristics in this service */
  }}
};
/*---------------------------------------------------------------------------*/
static uint16_t ble_srv_radio_handle;
static uint16_t ble_attr_radio_interval_handle;
static uint16_t ble_attr_radio_txpower_handle;
static uint16_t interval_handle_value = ADV_INTERVAL_LEVEL_1;
static int8_t txpower_handle_value = 0;
/*---------------------------------------------------------------------------*/
static BleGattService g_ble_gatt_alfa_radio_service = {
  .type = BLE_GATT_SERVICE_TYPE_PRIMARY,
  .uuid = &gatt_alfa_radio_service_uuid,
  .handle = &ble_srv_radio_handle,
  .characteristic_count = 2,
  .characteristics = (BleGattCharacteristic[]) { {
    .uuid = &gatt_alfa_radio_chr_interval,
    .value_handle = &ble_attr_radio_interval_handle,
    .props = BLE_GATT_CHR_PROPS_READ | BLE_GATT_CHR_PROPS_WRITE | BLE_GATT_CHR_PROPS_WRITE_NO_RSP,
    .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
    .init_value = &interval_handle_value,
    .init_value_len = 2,
  }, {
    .uuid = &gatt_alfa_radio_chr_tx_power,
    .value_handle = &ble_attr_radio_txpower_handle,
    .props = BLE_GATT_CHR_PROPS_READ | BLE_GATT_CHR_PROPS_WRITE | BLE_GATT_CHR_PROPS_WRITE_NO_RSP,
    .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
    .init_value = &txpower_handle_value,
    .init_value_len = 1,
  }, {
    0, /* End: No more characteristics in this service */
  }}
};
/*---------------------------------------------------------------------------*/
#if defined(BOARD_ALFA2477S)
static uint16_t ble_srv_alfa_2477s_handle;
static uint16_t ble_attr_alfa_2477s_rfatte_handle;
static uint16_t ble_attr_alfa_2477s_button_handle;
static uint16_t ble_attr_alfa_2477s_button_cccdhandle;
static uint16_t ble_attr_alfa_2477s_buzzer_handle;
static uint16_t ble_attr_alfa_2477s_led_handle;
static uint8_t rfatte_handle_value = 0x00;
static uint8_t buzzer_handle_value = 0x00;
static uint8_t led_handle_value = 0x00;
/*---------------------------------------------------------------------------*/
static BleGattService g_ble_gatt_alfa_2477s_service = {
  .type = BLE_GATT_SERVICE_TYPE_PRIMARY,
  .uuid = &gatt_alfa_2477s_service_uuid,
  .handle = &ble_srv_alfa_2477s_handle,
  .characteristic_count = 4,
  .characteristics = (BleGattCharacteristic[]) { {
    .uuid = &gatt_alfa_2477s_chr_rfatte,
    .value_handle = &ble_attr_alfa_2477s_rfatte_handle,
    .props = BLE_GATT_CHR_PROPS_READ | BLE_GATT_CHR_PROPS_WRITE | BLE_GATT_CHR_PROPS_WRITE_NO_RSP,
    .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
    .init_value = &rfatte_handle_value,
    .init_value_len = 1,
  }, {
    .uuid = &gatt_alfa_2477s_chr_button,
    .value_handle = &ble_attr_alfa_2477s_button_handle,
    .cccd_handle = &ble_attr_alfa_2477s_button_cccdhandle,
    .props = BLE_GATT_CHR_PROPS_NOTIFY,
    .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
    .init_value_len = 0,
  }, {
    .uuid = &gatt_alfa_2477s_chr_buzzer,
    .value_handle = &ble_attr_alfa_2477s_buzzer_handle,
    .props = BLE_GATT_CHR_PROPS_READ | BLE_GATT_CHR_PROPS_WRITE | BLE_GATT_CHR_PROPS_WRITE_NO_RSP,
    .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
    .init_value = &buzzer_handle_value,
    .init_value_len = 1,
  }, {
    .uuid = &gatt_alfa_2477s_chr_led,
    .value_handle = &ble_attr_alfa_2477s_led_handle,
    .props = BLE_GATT_CHR_PROPS_READ | BLE_GATT_CHR_PROPS_WRITE | BLE_GATT_CHR_PROPS_WRITE_NO_RSP,
    .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
    .init_value = &led_handle_value,
    .init_value_len = 1,
  }, {
    0, /* End: No more characteristics in this service */
  }}
};
#endif
/*---------------------------------------------------------------------------*/
static void setup_advertisement(void);
static void ble_gap_conn_evt_handler(uint16_t conn_handle, uint16_t state);
static int upsert_storage_data(const uint32_t key, uint8_t *data, uint32_t len);
static int qsert_storage_data(const uint32_t key, uint8_t *data, uint32_t len);
/*---------------------------------------------------------------------------*/
static int
upsert_storage_data(const uint32_t key, uint8_t *data, uint32_t len)
{
  int16_t fd;
  logger->printf(LOG_RTT, "[app] upsert key 0x%08X len %d\n", key, len);

  int ret = fileio->open(&fd, key, "w+");
  logger->printf(LOG_RTT, "[app] upsert open %d\n", ret);
  if (ret == ENONE) {
    // write default ibeacon uuid
    logger->printf(LOG_RTT, "[app] upsert write file len %d\n", len);
    ret = fileio->write(fd, data, len);
    logger->printf(LOG_RTT, "[app] upsert write file ret %d\n", ret);
  }
  fileio->close(fd);
  return ret;
}
/*---------------------------------------------------------------------------*/
static int
qsert_storage_data(const uint32_t key, uint8_t *data, uint32_t len)
{
  int16_t fd;
  int ret;

  int ret_size = fileio->size(key);
  logger->printf(LOG_RTT, "[app] qsert key 0x%08X ret_size %d\n", key, ret_size);

  if (ret_size) {
    logger->printf(LOG_RTT, "[app] qsert exist, read %d\n", ret_size);
    ret = fileio->open(&fd, key, "r");
    if (ret == ENONE) {
      // read 16 bytes for ibeacon uuid
      ret = fileio->read(fd, data, ret_size);
    }
    fileio->close(fd);
  } else {
    logger->printf(LOG_RTT, "[app] qsert create.\n");
    ret = fileio->open(&fd, key, "w+");
    if (ret == ENONE) {
      // write default ibeacon uuid
      ret = fileio->write(fd, data, len);
    }
    fileio->close(fd);
  }
  return ret;
}
/*---------------------------------------------------------------------------*/
static void
ibeacon_ble_write_evt_handler(uint16_t handle, uint8_t *value, uint16_t length)
{
  int ret;
  if (handle == ble_attr_ibeacon_uuid_handle) {
    memcpy(&ble_adv_manu_ibeacon_data[2], value, 16);
    ret = upsert_storage_data(FILE_KEY_IBEACON_UUID, &ble_adv_manu_ibeacon_data[2], 16);
    if (ret != ENONE) {
      logger->printf(LOG_RTT, "[app] update uuid error %d\n", ret);
    }
  } else if (handle == ble_attr_ibeacon_major_handle) {
    memcpy(&ble_adv_manu_ibeacon_data[18], value, 2);
    ret = upsert_storage_data(FILE_KEY_IBEACON_MAJOR, &ble_adv_manu_ibeacon_data[18], 2);
    if (ret != ENONE) {
      logger->printf(LOG_RTT, "[app] update major error %d\n", ret);
    }
  } else if (handle == ble_attr_ibeacon_minor_handle) {
    memcpy(&ble_adv_manu_ibeacon_data[20], value, 2);
    ret = upsert_storage_data(FILE_KEY_IBEACON_MINOR, &ble_adv_manu_ibeacon_data[20], 2);
    if (ret != ENONE) {
      logger->printf(LOG_RTT, "[app] update minor error %d\n", ret);
    }
  } else if (handle == ble_attr_ibeacon_txm_handle) {
    ble_adv_manu_ibeacon_data[22] = value[0];
    ret = upsert_storage_data(FILE_KEY_IBEACON_TXM, &ble_adv_manu_ibeacon_data[22], 1);
    if (ret != ENONE) {
      logger->printf(LOG_RTT, "[app] update txm error %d\n", ret);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
radio_ble_write_evt_handler(uint16_t handle, uint8_t *value, uint16_t length)
{
  int ret;
  if (handle == ble_attr_radio_interval_handle) {
    interval_handle_value = 0;
    interval_handle_value = value[1];
    interval_handle_value = interval_handle_value << 8;
    interval_handle_value = interval_handle_value | value[0];
    logger->printf(LOG_RTT, "[app] update radio interval %d\n", interval_handle_value);
    // memcpy(&interval_handle_value, value, 2);
    ret = upsert_storage_data(FILE_KEY_RADIO_INTERVAL, &interval_handle_value, 2);
    if (ret != ENONE) {
      logger->printf(LOG_RTT, "[app] update radio interval error %d\n", ret);
    }
  } else if (handle == ble_attr_radio_txpower_handle) {
    memcpy(&txpower_handle_value, value, 1);
    logger->printf(LOG_RTT, "[app] update radio txpower %d\n", txpower_handle_value);
    ret = upsert_storage_data(FILE_KEY_RADIO_TXPOWER, &txpower_handle_value, 1);
    if (ret != ENONE) {
      logger->printf(LOG_RTT, "[app] update radio txpower error %d\n", ret);
    }
    ble_manager->setTxPower(txpower_handle_value);
  }
}
/*---------------------------------------------------------------------------*/
#if defined(BOARD_ALFA2477S)
static void
alfa2477s_write_evt_handler(uint16_t handle, uint8_t *value, uint16_t length)
{
  int ret;
  uint8_t led_params;
  if (handle == ble_attr_alfa_2477s_rfatte_handle) {
    if (length == 1) {
      rfatte_handle_value = value[0];
      logger->printf(LOG_RTT, "[app] update alfa2477s rfatte %d\n", rfatte_handle_value);
      ret = upsert_storage_data(FILE_KEY_2477S_RF_ATTE, &rfatte_handle_value, 1);
      if (ret != ENONE) {
        logger->printf(LOG_RTT, "[app] update rfatte error %d\n", ret);
      }
      hwdriver_rfatte->write(&rfatte_handle_value, 1, 0);
    }
  } else if (handle == ble_attr_alfa_2477s_button_cccdhandle) {
    // TODO:
  } else if (handle == ble_attr_alfa_2477s_buzzer_handle) {
    // TODO:
  } else if (handle == ble_attr_alfa_2477s_led_handle) {
    // TODO:
    if (length == 1) {
      // led blink
      if (value[0] == 0x01) {
        led_params = 1;
        hwdriver_led->write(&led_params, 1, 0);
        hwdriver_led->write(&led_params, 1, 1);
        hwdriver_led->write(&led_params, 1, 2);
        process->delay(20);
        led_params = 0;
        hwdriver_led->write(&led_params, 1, 0);
        hwdriver_led->write(&led_params, 1, 1);
        hwdriver_led->write(&led_params, 1, 2);
      }
    }
  }
}
#endif
/*---------------------------------------------------------------------------*/
static void
ble_gatt_chr_write_req(uint16_t conn_handle, uint16_t handle, uint8_t *value, uint16_t length)
{
  int len = length;
  if (conn_handle == peripheral_conn_handle) {
    ibeacon_ble_write_evt_handler(handle, value, length);
    radio_ble_write_evt_handler(handle, value, length);
#if defined(BOARD_ALFA2477S)
    alfa2477s_write_evt_handler(handle, value, length);
#endif
    setup_advertisement();
  }
}
/*---------------------------------------------------------------------------*/
static void
setup_advertisement(void)
{
  uint16_t service_uuid = 0xA55A;

  // setup the data
  adv_builder->init(&advdata);

  // add the iBeacon
  adv_builder->addManufacturerData(&advdata, DEFAULT_IBEACON_COMPANY_IDENTIFIER, ble_adv_manu_ibeacon_data, DEFAULT_IBEACON_BEACON_INFO_LENGTH);

  // initiate scan response packets
  adv_builder->initScanRsp(&scan_rsp_advdata);

  // Add the completed local name
  adv_builder->addCompleteLocalName(&scan_rsp_advdata, device_name, strlen(device_name));

  adv_builder->addService16bitUUID(&scan_rsp_advdata, service_uuid);

  // Add the service uuid and data
#if defined(BOARD_ALFAAA)
  service_data_packetes[0] = 0x00;
#elif defined(BOARD_ALFA2477)
  service_data_packetes[0] = 0x01;
#elif defined(BOARD_ALFAUSB)
  service_data_packetes[0] = 0x02;
#elif defined(BOARD_ALFA2477S)
  service_data_packetes[0] = 0x03;
#endif
  service_data_packetes[1] = device->getBatteryLevel();
  service_data_packetes[2] = ble_adv_manu_ibeacon_data[18];
  service_data_packetes[3] = ble_adv_manu_ibeacon_data[19];
  service_data_packetes[4] = ble_adv_manu_ibeacon_data[20];
  service_data_packetes[5] = ble_adv_manu_ibeacon_data[21];
  service_data_packetes[6] = ble_adv_manu_ibeacon_data[22];
  // service_data_packetes[2] = m_mac_address[5];
  // service_data_packetes[3] = m_mac_address[4];
  // service_data_packetes[4] = m_mac_address[3];
  // service_data_packetes[5] = m_mac_address[2];
  // service_data_packetes[6] = m_mac_address[1];
  // service_data_packetes[7] = m_mac_address[0];
  adv_builder->addServiceData(&scan_rsp_advdata, service_uuid, &service_data_packetes[0], 7);

  // setup the advertisement
  ble_manager->setAdvertisementData(&advdata, &scan_rsp_advdata);
}
/*---------------------------------------------------------------------------*/
static void
adv_sendout_callback(void)
{
  adv_sendout_counter++;
  if (adv_sendout_counter > 10000) {
    adv_sendout_counter = 0;
    setup_advertisement();
  }
}
/*---------------------------------------------------------------------------*/
const static BleGattServerCallback callback = {
  .onCharacteristicWriteRequest = ble_gatt_chr_write_req,
  .onCharacteristicReadRequest = NULL,
  .onConnectionStateChanged = ble_gap_conn_evt_handler
};
/*---------------------------------------------------------------------------*/
const static BleAdvertiseCallback adv_callback = {
  .onSendout = adv_sendout_callback
};
/*---------------------------------------------------------------------------*/
static void
ble_gap_conn_evt_handler(uint16_t conn_handle, uint16_t state)
{
  peripheral_conn_handle = conn_handle;
  if (state == BLE_GATT_STATE_CONNECTED) {
    is_connected = true;
    ble_manager->stopAdvertising();
  } else if (state == BLE_GATT_STATE_DISCONNECTED) {
    is_connected = false;
    ble_manager->startAdvertising(interval_handle_value, &adv_callback, &callback);
  }
}
/*---------------------------------------------------------------------------*/
int main(void)
{
  int errcode;
  int16_t fd;
  int key1_size;

  // Get Framework API Instance
  process = OSProcess();
  logger = OSLogger();
  ble_manager = CKBleManager();
  adv_builder = CKAdvDataBuilder();
  device = OSDevice();
  fileio = OSFileIO();

#if defined(BOARD_ALFA2477S)
  hwdriver_rfatte = HWPipe("rf_atte");
  hwdriver_led = HWPipe("led");
#endif

  // Get the mac address
  ble_manager->getMacAddress(m_mac_address);
  ble_manager->setDeviceName(device_name, strlen(device_name));

  // Get default parameters: iBeacon UUID
  errcode = qsert_storage_data(FILE_KEY_IBEACON_UUID, &ble_adv_manu_ibeacon_data[2], 16);
  if (errcode != ENONE) {
    logger->printf(LOG_RTT,"[app] get uuid error %d\n", errcode);
    return 0;
  }

  // Get default parameters: iBeacon Minor
  errcode = qsert_storage_data(FILE_KEY_IBEACON_MAJOR, &ble_adv_manu_ibeacon_data[18], 2);
  if (errcode != ENONE) {
    logger->printf(LOG_RTT,"[app] get major error %d\n", errcode);
    return 0;
  }

  // Get default parameters: iBeacon Minor
  errcode = qsert_storage_data(FILE_KEY_IBEACON_MINOR, &ble_adv_manu_ibeacon_data[20], 2);
  if (errcode != ENONE) {
    logger->printf(LOG_RTT,"[app] get minor error %d\n", errcode);
    return 0;
  }

  // Get default parameters: iBeacon Txm
  errcode = qsert_storage_data(FILE_KEY_IBEACON_TXM, &ble_adv_manu_ibeacon_data[22], 1);
  if (errcode != ENONE) {
    logger->printf(LOG_RTT,"[app] get txm error %d\n", errcode);
    return 0;
  }

  // Get default parameters: Radio interval
  errcode = qsert_storage_data(FILE_KEY_RADIO_INTERVAL, (uint8_t*)&interval_handle_value, 2);
  if (errcode != ENONE) {
    logger->printf(LOG_RTT,"[app] get radio interval error %d\n", errcode);
    return 0;
  }

  // Get default parameters: Radio interval
  errcode = qsert_storage_data(FILE_KEY_RADIO_TXPOWER, &txpower_handle_value, 1);
  if (errcode != ENONE) {
    logger->printf(LOG_RTT,"[app] get radio txpower error %d\n", errcode);
    return 0;
  }

  // Get default parameters:  2477s rf atte
#if defined(BOARD_ALFA2477S)
  errcode = qsert_storage_data(FILE_KEY_2477S_RF_ATTE, &rfatte_handle_value, 1);
  if (errcode != ENONE) {
    logger->printf(LOG_RTT,"[app] get rf atte error %d\n", errcode);
    return 0;
  }
  hwdriver_rfatte->write(&rfatte_handle_value, 1, 0);
#endif

  // setup the tx power
  ble_manager->setTxPower(txpower_handle_value);

  setup_advertisement();

  errcode = ble_manager->addService(&g_ble_gatt_alfa_ibeacon_service);
  if (errcode != ENONE) {
    logger->printf(LOG_RTT,"[app] add service error %d\n", errcode);
    return 0;
  }

  errcode = ble_manager->addService(&g_ble_gatt_alfa_radio_service);
  if (errcode != ENONE) {
    logger->printf(LOG_RTT,"[app] add service error %d\n", errcode);
    return 0;
  }

#if defined(BOARD_ALFA2477S)
  errcode = ble_manager->addService(&g_ble_gatt_alfa_2477s_service);
  if (errcode != ENONE) {
    logger->printf(LOG_RTT,"[app] add service error %d\n", errcode);
    return 0;
  }
#endif

  logger->printf(LOG_RTT,"[app] startAdvertising interval %d\n", interval_handle_value);

  // start advertising
  errcode = ble_manager->startAdvertising(interval_handle_value, &adv_callback, &callback);
  if (errcode != ENONE) {
    logger->printf(LOG_RTT,"[app] startAdvertising error %d\n", errcode);
    return 0;
  }

  while (1) {
    // Power saving
    process->waitForEvent();
  }
  return 0;
}
