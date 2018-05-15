#include "alfabase.h"

/* LINE Simple Beacon Profile {A78D0001-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid128 gatt_alfa_lsb_service_uuid =
    BLE_UUID128_INIT(0x97, 0x12, 0xa3, 0xbc, 0x8a, 0x12, 0xF9, 0xE9,
                     0xa3, 0x09, 0xc2, 0xF6, 0x01, 0x00, 0x8D, 0xA7);
/* LINE Simple Beacon HWID {A78D0002-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfa_lsb_chr_hwid = BLE_UUID16_INIT(0x0002);

/* LINE Simple Beacon device message {A78D0003-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfa_lsb_chr_dm = BLE_UUID16_INIT(0x0003);

const static uint32_t FILE_KEY_LSB_HWID = 0x00000001;
const static uint32_t FILE_KEY_LSB_DM = 0x00000002;

static Logger *logger;
static AdvDataBuilder *adv_builder;
static BleManager *ble_manager;
static Device *device;
static FileIO *fileio;
static AdvData advdata;
static AdvData scan_rsp_advdata;

static char *device_name = "Simple";

static uint16_t peripheral_conn_handle;
static bool is_connected = false;
static bool is_notification_enabled = false;
static bool adv_event_flag = false;

/* ble service handle */
static uint16_t ble_service_handle;
static uint16_t ble_attr_lsb_chr_hwid_handle;
static uint16_t ble_attr_lsb_chr_dm_handle;
static uint16_t peripheral_conn_handle;

#define DEAULT_HWID 0x32, 0xAF, 0x51, 0x9E, 0x88
#define DEFAULT_DEVICE_MESSAGE 'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '!', '!'
static uint8_t simplebeacon_packet[20] =
                          {
                            0x02, // SimpleBeacon
                            DEAULT_HWID,
                            0x7F, // Measured TX power
                            DEFAULT_DEVICE_MESSAGE,
                          };
/*---------------------------------------------------------------------------*/
static BleGattService g_ble_gatt_alfa_lsb_service = {
  .type = BLE_GATT_SERVICE_TYPE_PRIMARY,
  .uuid = &gatt_alfa_lsb_service_uuid,
  .handle = &ble_service_handle,
  .characteristic_count = 2,
  .characteristics = (BleGattCharacteristic[]) { {
      .uuid = &gatt_alfa_lsb_chr_hwid,
      .value_handle = &ble_attr_lsb_chr_hwid_handle,
      .props = BLE_GATT_CHR_PROPS_READ | BLE_GATT_CHR_PROPS_WRITE | BLE_GATT_CHR_PROPS_WRITE_NO_RSP,
      .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
      .init_value = &simplebeacon_packet[1],
      .init_value_len = 5,
  }, {
      /* Characteristic: Write */
      .uuid = &gatt_alfa_lsb_chr_dm,
      .value_handle = &ble_attr_lsb_chr_dm_handle,
      .props = BLE_GATT_CHR_PROPS_READ | BLE_GATT_CHR_PROPS_WRITE | BLE_GATT_CHR_PROPS_WRITE_NO_RSP,
      .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
      .init_value = &simplebeacon_packet[7],
      .init_value_len = 13,
  }, {
    0, /* End: No more characteristics in this service */
  }}
};
/*---------------------------------------------------------------------------*/
static void
ble_gatt_charact_write_req(uint16_t conn_handle, uint16_t handle, uint8_t *value, uint16_t length)
{
  int len = length;
  logger->printf(LOG_RTT, "[app] write length %d\n", len);
  if (len > 20) {
    return;
  }

  // data from peer
  if (handle == ble_attr_lsb_chr_dm_handle) {

  } else if (handle == ble_attr_lsb_chr_hwid_handle) {
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
  .onCharacteristicWriteRequest = ble_gatt_charact_write_req,
  .onCharacteristicReadRequest = NULL,
  .onConnectionStateChanged = ble_gap_conn_evt_handler
};
/****************************************************************/
int main(void)
{
  int errcode;
  int16_t fd;
  Process *process = OSProcess();
  logger = OSLogger();
  ble_manager = CKBleManager();
  adv_builder = CKAdvDataBuilder();
  device = OSDevice();
  fileio = OSFileIO();

  // Get default parameters: simple beacon hwid
  int ret_size = fileio->size(FILE_KEY_LSB_HWID);
  if (ret_size) {
    int ret = fileio->open(&fd, FILE_KEY_LSB_HWID, "r");
    if (ret == ENONE) {
      // read 5 bytes for hwid
      ret = fileio->read(fd, &simplebeacon_packet[1], ret_size);
    }
    fileio->close(fd);
  } else {
    int ret = fileio->open(&fd, FILE_KEY_LSB_HWID, "w+");
    if (ret == ENONE) {
      // write default hwid
      ret = fileio->write(fd, &simplebeacon_packet[1], 5);
      if (ret == ENONE) {
        logger->printf(LOG_RTT, "[app] init 5 bytes to hwid\n");
      }
      fileio->close(fd);
    }
  }

  // Get default parameters: simple beacon device message
  ret_size = fileio->size(FILE_KEY_LSB_DM);
  if (ret_size) {
    int ret = fileio->open(&fd, FILE_KEY_LSB_DM, "r");
    if (ret == ENONE) {
      // read 13 bytes for device message
      ret = fileio->read(fd, &simplebeacon_packet[7], ret_size);
    }
    fileio->close(fd);
  } else {
    int ret = fileio->open(&fd, FILE_KEY_LSB_DM, "w+");
    if (ret == ENONE) {
      // write default device message
      ret = fileio->write(fd, &simplebeacon_packet[7], 13);
      if (ret == ENONE) {
        logger->printf(LOG_RTT, "[app] init 13 bytes to device message\n");
      }
      fileio->close(fd);
    }
  }

  // setup the ibeacon data
  adv_builder->init(&advdata);
  uint16_t line_crop_uuid = 0xFE6F;
  adv_builder->addService16bitUUID(&advdata, line_crop_uuid);
  adv_builder->addServiceData(&advdata, line_crop_uuid, &simplebeacon_packet[0], 20);

  adv_builder->initScanRsp(&scan_rsp_advdata);
  adv_builder->addCompleteLocalName(&scan_rsp_advdata, device_name, strlen(device_name));

  errcode = ble_manager->setAdvertisementData(&advdata, &scan_rsp_advdata);
  if (errcode != ENONE) {
    logger->printf(LOG_RTT, "[app] setAdvertisementData error %d\n", errcode);
    return 0;
  }

  errcode = ble_manager->addService(&g_ble_gatt_alfa_lsb_service);
  if (errcode != ENONE) {
    logger->printf(LOG_RTT,"[app] addService error %d\n", errcode);
    return 0;
  }

  errcode = ble_manager->startAdvertising(ADV_INTERVAL_LEVEL_0, NULL, &callback);
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
