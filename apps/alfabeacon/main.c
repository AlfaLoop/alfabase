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

#define DEFAULT_IBEACON_COMPANY_IDENTIFIER          0x004C                            /**< Company identifier for Nordic Semiconductor ASA. as per www.bluetooth.org. */

#define DEFAULT_IBEACON_BEACON_INFO_LENGTH          0x17                              /**< Total length of information advertised by the Beacon. */
#define DEFAULT_IBEACON_DEVICE_TYPE                 0x02                              /**< 0x02 refers to Beacon. */
#define DEFAULT_IBEACON_ADV_DATA_LENGTH             0x15                              /**< Length of manufacturer specific data in the advertisement. */
#define DEFAULT_IBEACON_UUID            0x15, 0x34, 0x51, 0x64, \
                                        0x67, 0xAB, 0x3E, 0x49, \
                                        0xF9, 0xD6, 0xE2, 0x90, \
                                        0x00, 0x00, 0x00, 0x08            /**< Proprietary UUID for Beacon. */
#define DEFAULT_IBEACON_MAJOR_VALUE                 0x17, 0x74                        /**< Major value used to identify Beacons. */
#define DEFAULT_IBEACON_MINOR_VALUE                 0x00, 0x09                        /**< Minor value used to identify Beacons. */
#define DEFAULT_IBEACON_MEASURED_RSSI               0xC3                              /**< The Beacon's measured RSSI at 1 meter distance in dBm. */

// AlfaBeacon iBeacon Profile
/* {A78E0001-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid128 gatt_alfa_ibeacon_service_uuid =
    BLE_UUID128_INIT(0x97, 0x12, 0xa3, 0xbc, 0x8a, 0x12, 0xF9, 0xE9,
                     0xa3, 0x09, 0xc2, 0xF6, 0x01, 0x00, 0x8E, 0xA7);
/* iBeacon UUID {A78E0002-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfa_ibeacon_chr_uuid = BLE_UUID16_INIT(0x0002);

/* iBeacon Major {A78E0003-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfa_ibeacon_chr_major = BLE_UUID16_INIT(0x0003);

/* iBeacon Minor {A78E0004-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfa_ibeacon_chr_minor = BLE_UUID16_INIT(0x0004);

/* iBeacon Minor {A78E0005-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfa_ibeacon_chr_txm = BLE_UUID16_INIT(0x0005);

const static uint32_t FILE_KEY_IBEACON_UUID = 0x00000001;
const static uint32_t FILE_KEY_IBEACON_MAJOR = 0x00000002;
const static uint32_t FILE_KEY_IBEACON_MINOR = 0x00000003;
const static uint32_t FILE_KEY_IBEACON_TXM = 0x00000004;

static Logger *logger;
static AdvDataBuilder *adv_builder;
static BleManager *ble_manager;
static Device *device;
static AdvData advdata;
static AdvData scan_rsp_advdata;
static FileIO *fileio;

/* ble ibeacon service handle */
static uint16_t ble_srv_ibeacon_handle;
static uint16_t ble_attr_ibeacon_uuid_handle;
static uint16_t ble_attr_ibeacon_major_handle;
static uint16_t ble_attr_ibeacon_minor_handle;
static uint16_t ble_attr_ibeacon_txm_handle;
static uint8_t service_data_packetes[8];

static uint8_t tx_buffer[BLE_GATT_CHR_MAX_SIZE];

/*---------------------------------------------------------------------------*/
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
    DEFAULT_IBEACON_MEASURED_RSSI    // Manufacturer specific information. The Beacon's measured TX power in
                         // this implementation.
};

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
const static char *device_name = "AlfaBeacon";
static uint8_t m_mac_address[6];
static uint16_t peripheral_conn_handle;
static bool is_connected = false;


/*---------------------------------------------------------------------------*/
static void setup_advertisement(void);
/*---------------------------------------------------------------------------*/
static void
ble_ibeacon_update_uuid(uint8_t *value)
{
  memcpy(&ble_adv_manu_ibeacon_data[2], value, 16);
  int16_t fd;
  int ret = fileio->open(&fd, FILE_KEY_IBEACON_UUID, "w");
  if (ret == ENONE) {
    // write default ibeacon uuid
    ret = fileio->write(fd, &ble_adv_manu_ibeacon_data[2], 16);
    if (ret == ENONE) {
      logger->printf(LOG_RTT, "[app] update uuid\n");
    }
    fileio->close(fd);
  }
}
/*---------------------------------------------------------------------------*/
static void
ble_ibeacon_update_major(uint8_t *value)
{
  memcpy(&ble_adv_manu_ibeacon_data[18], value, 2);
  int16_t fd;
  int ret = fileio->open(&fd, FILE_KEY_IBEACON_MAJOR, "w");
  if (ret == ENONE) {
    // write default ibeacon uuid
    ret = fileio->write(fd, &ble_adv_manu_ibeacon_data[18], 2);
    if (ret == ENONE) {
      logger->printf(LOG_RTT, "[app] update major\n");
    }
    fileio->close(fd);
  }
}
/*---------------------------------------------------------------------------*/
static void
ble_ibeacon_update_minor(uint8_t *value)
{
  memcpy(&ble_adv_manu_ibeacon_data[20], value, 2);
  int16_t fd;
  int ret = fileio->open(&fd, FILE_KEY_IBEACON_MINOR, "w");
  if (ret == ENONE) {
    // write default ibeacon uuid
    ret = fileio->write(fd, &ble_adv_manu_ibeacon_data[20], 2);
    if (ret == ENONE) {
      logger->printf(LOG_RTT, "[app] update minor\n");
    }
    fileio->close(fd);
  }
}
/*---------------------------------------------------------------------------*/
static void
ble_ibeacon_update_txm(uint8_t value)
{
  ble_adv_manu_ibeacon_data[22] = value;
  int16_t fd;
  int ret = fileio->open(&fd, FILE_KEY_IBEACON_TXM, "w");
  if (ret == ENONE) {
    // write default ibeacon uuid
    ret = fileio->write(fd, &ble_adv_manu_ibeacon_data[22], 1);
    if (ret == ENONE) {
      logger->printf(LOG_RTT, "[app] update txm\n");
    }
    fileio->close(fd);
  }
}
/*---------------------------------------------------------------------------*/
static void
ble_gatt_chr_write_req(uint16_t conn_handle, uint16_t handle, uint8_t *value, uint16_t length)
{
  int len = length;
  if (conn_handle == peripheral_conn_handle) {
    if (handle == ble_attr_ibeacon_uuid_handle) {
      logger->printf(LOG_RTT, "[app] update uuid len %d\n", length);
      ble_ibeacon_update_uuid(value);
    } else if (handle == ble_attr_ibeacon_major_handle) {
      logger->printf(LOG_RTT, "[app] update major len %d\n", length);
      ble_ibeacon_update_major(value);
    } else if (handle == ble_attr_ibeacon_minor_handle) {
      logger->printf(LOG_RTT, "[app] update minor len %d\n", length);
      ble_ibeacon_update_minor(value);
    } else if (handle == ble_attr_ibeacon_txm_handle) {
      logger->printf(LOG_RTT, "[app] update txm len %d\n", length);
      ble_ibeacon_update_txm(*value);
    }
    setup_advertisement();
  }
}
/*---------------------------------------------------------------------------*/
static void
ble_gap_conn_evt_handler(uint16_t conn_handle, uint16_t state)
{
  peripheral_conn_handle = conn_handle;
  if (state == BLE_GATT_STATE_CONNECTED) {
    is_connected = true;
  } else if (state == BLE_GATT_STATE_DISCONNECTED) {
    is_connected = false;
  }
}
/*---------------------------------------------------------------------------*/
const static BleGattServerCallback callback = {
  .onCharacteristicWriteRequest = ble_gatt_chr_write_req,
  .onCharacteristicReadRequest = NULL,
  .onConnectionStateChanged = ble_gap_conn_evt_handler
};
/****************************************************************/
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
  service_data_packetes[0] = 0x00;  // device type
  service_data_packetes[1] = 0x64;  // battery level
  service_data_packetes[2] = m_mac_address[5];
  service_data_packetes[3] = m_mac_address[4];
  service_data_packetes[4] = m_mac_address[3];
  service_data_packetes[5] = m_mac_address[2];
  service_data_packetes[6] = m_mac_address[1];
  service_data_packetes[7] = m_mac_address[0];
  adv_builder->addServiceData(&scan_rsp_advdata, service_uuid, &service_data_packetes[0], 8);

  // setup the advertisement
  ble_manager->setAdvertisementData(&advdata, &scan_rsp_advdata);
}
/****************************************************************/
int main(void)
{
  int errcode;
  int16_t fd;
  int key1_size;

  // Get Framework API Instance
  Process *process = OSProcess();
  logger = OSLogger();
  ble_manager = CKBleManager();
  adv_builder = CKAdvDataBuilder();
  device = OSDevice();
  fileio = OSFileIO();

  // Get the mac address
  ble_manager->getMacAddress(m_mac_address);
  ble_manager->setDeviceName(device_name, strlen(device_name));

  // Get default parameters: iBeacon UUID
  int ret_size = fileio->size(FILE_KEY_IBEACON_UUID);
  if (ret_size) {
    int ret = fileio->open(&fd, FILE_KEY_IBEACON_UUID, "r");
    if (ret == ENONE) {
      // read 16 bytes for ibeacon uuid
      ret = fileio->read(fd, &ble_adv_manu_ibeacon_data[2], ret_size);
    }
    fileio->close(fd);
  } else {
    int ret = fileio->open(&fd, FILE_KEY_IBEACON_UUID, "w+");
    if (ret == ENONE) {
      // write default ibeacon uuid
      ret = fileio->write(fd, &ble_adv_manu_ibeacon_data[2], 16);
      if (ret == ENONE) {
        logger->printf(LOG_RTT, "[app] init 16 bytes to uuid\n");
      }
      fileio->close(fd);
    }
  }

  // Get default parameters: iBeacon Minor
  ret_size = fileio->size(FILE_KEY_IBEACON_MAJOR);
  if (ret_size) {
    int ret = fileio->open(&fd, FILE_KEY_IBEACON_MAJOR, "r");
    if (ret == ENONE) {
      // read 16 bytes for ibeacon uuid
      ret = fileio->read(fd, &ble_adv_manu_ibeacon_data[18], ret_size);
    }
    fileio->close(fd);
  } else {
    int ret = fileio->open(&fd, FILE_KEY_IBEACON_MAJOR, "w+");
    if (ret == ENONE) {
      // write default ibeacon uuid
      ret = fileio->write(fd, &ble_adv_manu_ibeacon_data[18], 2);
      if (ret == ENONE) {
        logger->printf(LOG_RTT, "[app] init 2 bytes to major\n");
      }
      fileio->close(fd);
    }
  }

  // Get default parameters: iBeacon Minor
  ret_size = fileio->size(FILE_KEY_IBEACON_MINOR);
  if (ret_size) {
    int ret = fileio->open(&fd, FILE_KEY_IBEACON_MINOR, "r");
    if (ret == ENONE) {
      // read 16 bytes for ibeacon uuid
      ret = fileio->read(fd, &ble_adv_manu_ibeacon_data[20], ret_size);
    }
    fileio->close(fd);
  } else {
    int ret = fileio->open(&fd, FILE_KEY_IBEACON_MINOR, "w+");
    if (ret == ENONE) {
      // write default ibeacon uuid
      ret = fileio->write(fd, &ble_adv_manu_ibeacon_data[20], 2);
      if (ret == ENONE) {
        logger->printf(LOG_RTT, "[app] init 2 bytes to minor\n");
      }
      fileio->close(fd);
    }
  }

  // Get default parameters: iBeacon Txm
  ret_size = fileio->size(FILE_KEY_IBEACON_TXM);
  if (ret_size) {
    int ret = fileio->open(&fd, FILE_KEY_IBEACON_TXM, "r");
    if (ret == ENONE) {
      // read 16 bytes for ibeacon uuid
      ret = fileio->read(fd, &ble_adv_manu_ibeacon_data[22], ret_size);
    }
    fileio->close(fd);
  } else {
    int ret = fileio->open(&fd, FILE_KEY_IBEACON_TXM, "w+");
    if (ret == ENONE) {
      // write default ibeacon uuid
      ret = fileio->write(fd, &ble_adv_manu_ibeacon_data[22], 1);
      if (ret == ENONE) {
        logger->printf(LOG_RTT, "[app] init a byte to txm\n");
      }
      fileio->close(fd);
    }
  }

  setup_advertisement();
  errcode = ble_manager->addService(&g_ble_gatt_alfa_ibeacon_service);
  if (errcode != ENONE) {
    logger->printf(LOG_RTT,"[app] add service error %d\n", errcode);
    return 0;
  }

  errcode = ble_manager->startAdvertising(ADV_INTERVAL_LEVEL_1, NULL, &callback);
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
