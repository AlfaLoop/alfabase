#include "alfabase.h"

// Crystal
#define BUTTON0                  2u
#define BUTTON1                  30u
#define BUZZER_PIN               3u
#define LED0                     4u
#define LED1                     11u
#define LED2                     31u
#define GPIO_EXT0                12u
#define GPIO_EXT1                13u
#define GPIO_EXT2                14u
#define GPIO_EXT3                15u
#define RADIO_ATT_16dbm          16u
#define RADIO_ATT_8dbm           17u
#define RADIO_ATT_4dbm           18u
#define RADIO_ATT_2dbm           19u
#define RADIO_ATT_1dbm           20u

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
static AdvData advdata;
static AdvData scan_rsp_advdata;
static Pin *pin;

static char *device_name = "AlfaLSB";
static uint8_t android_battery_packetes[3];
static uint8_t ios_battery_packetes[5];
static uint8_t beacon_major[2];
static uint8_t beacon_minor[2];

static uint16_t peripheral_conn_handle;
static bool is_connected = false;
static bool is_notification_enabled = false;
static bool adv_event_flag = false;
static uint8_t tx_buffer[BLE_GATT_CHR_MAX_SIZE];

static uint8_t
manufacturer_ibeacon_data[APP_BEACON_INFO_LENGTH] =                    /**< Information advertised by the Beacon. */
{
    APP_DEVICE_TYPE,     // Manufacturer specific information. Specifies the device type in this
                         // implementation.
    APP_ADV_DATA_LENGTH, // Manufacturer specific information. Specifies the length of the
                         // manufacturer specific data in this implementation.
    APP_BEACON_UUID,     // 128 bit UUID value.
    APP_MAJOR_VALUE,     // Major arbitrary value that can be used to distinguish between Beacons.
    APP_MINOR_VALUE,     // Minor arbitrary value that can be used to distinguish between Beacons.
    APP_MEASURED_RSSI    // Manufacturer specific information. The Beacon's measured TX power in
                         // this implementation.
};

/* ble service handle */
static uint16_t ble_service_handle;
static uint16_t ble_attr_rx_handle;
static uint16_t ble_attr_rx_cccdhandle;
static uint16_t ble_attr_tx_handle;
static uint16_t peripheral_conn_handle;
/*---------------------------------------------------------------------------*/
static BleGattService g_ble_gatt_nus_service = {
  .type = BLE_GATT_SERVICE_TYPE_PRIMARY,
  .uuid = &gatt_custom_service_uuid,
  .handle = &ble_service_handle,
  .characteristic_count = 2,
  .characteristics = (BleGattCharacteristic[]) { {
      .uuid = &gatt_custom_chr_tx_uuid,
      .value_handle = &ble_attr_rx_handle,
      .cccd_handle = &ble_attr_rx_cccdhandle,
      .props = BLE_GATT_CHR_PROPS_NOTIFY,
      .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
  }, {
      /* Characteristic: Write */
      .uuid = &gatt_custom_chr_rx_uuid,
      .value_handle = &ble_attr_tx_handle,
      .props = BLE_GATT_CHR_PROPS_WRITE | BLE_GATT_CHR_PROPS_WRITE_NO_RSP,
      .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
  }, {
    0, /* End: No more characteristics in this service */
  }}
};
/*---------------------------------------------------------------------------*/
static void
set_txpower(uint8_t power)
{
  logger->printf(LOG_RTT, "[app] set txpower ");
  switch (power) {
    case 0x30:
    ble_manager->setTxPower(4);
    logger->printf(LOG_RTT, "4dbm\n");
    break;
    case 0x31:
    ble_manager->setTxPower(0);
    logger->printf(LOG_RTT, "0dbm\n");
    break;
    case 0x32:
    ble_manager->setTxPower(-4);
    logger->printf(LOG_RTT, "-4dbm\n");
    break;
    case 0x33:
    ble_manager->setTxPower(-8);
    logger->printf(LOG_RTT, "-8dbm\n");
    break;
    case 0x34:
    ble_manager->setTxPower(-12);
    logger->printf(LOG_RTT, "-12dbm\n");
    break;
    case 0x35:
    ble_manager->setTxPower(-16);
    logger->printf(LOG_RTT, "-16dbm\n");
    break;
    case 0x36:
    ble_manager->setTxPower(-20);
    logger->printf(LOG_RTT, "-20dbm\n");
    break;
    case 0x37:
    ble_manager->setTxPower(-40);
    logger->printf(LOG_RTT, "-40dbm\n");
    break;
    default:
    ble_manager->setTxPower(0);
    logger->printf(LOG_RTT, "0dbm\n");
    break;
  }
}
/*---------------------------------------------------------------------------*/
static void
set_attenuator(uint8_t power)
{
  logger->printf(LOG_RTT, "[app] set attenuator ");
  switch (power) {
    case 0x31:
    pin->output(RADIO_ATT_1dbm, PIN_OUTPUT_HIGH);
    pin->output(RADIO_ATT_2dbm, PIN_OUTPUT_HIGH);
    pin->output(RADIO_ATT_4dbm, PIN_OUTPUT_HIGH);
    pin->output(RADIO_ATT_8dbm, PIN_OUTPUT_HIGH);
    pin->output(RADIO_ATT_16dbm, PIN_OUTPUT_HIGH);
    logger->printf(LOG_RTT, "0dbm\n");
    break;
    case 0x32:
    pin->output(RADIO_ATT_1dbm, PIN_OUTPUT_LOW);
    pin->output(RADIO_ATT_2dbm, PIN_OUTPUT_HIGH);
    pin->output(RADIO_ATT_4dbm, PIN_OUTPUT_HIGH);
    pin->output(RADIO_ATT_8dbm, PIN_OUTPUT_HIGH);
    pin->output(RADIO_ATT_16dbm, PIN_OUTPUT_HIGH);
    logger->printf(LOG_RTT, "1dbm\n");
    break;
    case 0x33:
    pin->output(RADIO_ATT_1dbm, PIN_OUTPUT_HIGH);
    pin->output(RADIO_ATT_2dbm, PIN_OUTPUT_LOW);
    pin->output(RADIO_ATT_4dbm, PIN_OUTPUT_HIGH);
    pin->output(RADIO_ATT_8dbm, PIN_OUTPUT_HIGH);
    pin->output(RADIO_ATT_16dbm, PIN_OUTPUT_HIGH);
    logger->printf(LOG_RTT, "2dbm\n");
    break;
    case 0x34:
    pin->output(RADIO_ATT_1dbm, PIN_OUTPUT_HIGH);
    pin->output(RADIO_ATT_2dbm, PIN_OUTPUT_HIGH);
    pin->output(RADIO_ATT_4dbm, PIN_OUTPUT_LOW);
    pin->output(RADIO_ATT_8dbm, PIN_OUTPUT_HIGH);
    pin->output(RADIO_ATT_16dbm, PIN_OUTPUT_HIGH);
    logger->printf(LOG_RTT, "4dbm\n");
    break;
    case 0x35:
    pin->output(RADIO_ATT_1dbm, PIN_OUTPUT_HIGH);
    pin->output(RADIO_ATT_2dbm, PIN_OUTPUT_HIGH);
    pin->output(RADIO_ATT_4dbm, PIN_OUTPUT_HIGH);
    pin->output(RADIO_ATT_8dbm, PIN_OUTPUT_LOW);
    pin->output(RADIO_ATT_16dbm, PIN_OUTPUT_HIGH);
    logger->printf(LOG_RTT, "8dbm\n");
    break;
    case 0x36:
    pin->output(RADIO_ATT_1dbm, PIN_OUTPUT_HIGH);
    pin->output(RADIO_ATT_2dbm, PIN_OUTPUT_HIGH);
    pin->output(RADIO_ATT_4dbm, PIN_OUTPUT_HIGH);
    pin->output(RADIO_ATT_8dbm, PIN_OUTPUT_HIGH);
    pin->output(RADIO_ATT_16dbm, PIN_OUTPUT_LOW);
    logger->printf(LOG_RTT, "16dbm\n");
    break;
    case 0x37:
    pin->output(RADIO_ATT_1dbm, PIN_OUTPUT_LOW);
    pin->output(RADIO_ATT_2dbm, PIN_OUTPUT_LOW);
    pin->output(RADIO_ATT_4dbm, PIN_OUTPUT_LOW);
    pin->output(RADIO_ATT_8dbm, PIN_OUTPUT_LOW);
    pin->output(RADIO_ATT_16dbm, PIN_OUTPUT_LOW);
    logger->printf(LOG_RTT, "31dbm\n");
    break;
    default:
    pin->output(RADIO_ATT_1dbm, PIN_OUTPUT_HIGH);
    pin->output(RADIO_ATT_2dbm, PIN_OUTPUT_HIGH);
    pin->output(RADIO_ATT_4dbm, PIN_OUTPUT_HIGH);
    pin->output(RADIO_ATT_8dbm, PIN_OUTPUT_HIGH);
    pin->output(RADIO_ATT_16dbm, PIN_OUTPUT_HIGH);
    logger->printf(LOG_RTT, "0dbm\n");
    break;
  }
}
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
  if (handle == ble_attr_tx_handle) {
    logger->printf(LOG_RTT, "data ");
    for (int i = 0; i < length; i++) {
      logger->printf(LOG_RTT, "0x%02x ", value[i]);
    }
    logger->printf(LOG_RTT, "\n");

    if (length == 3) {
      if (value[0] == 0x31) {
        set_txpower(value[1]);
        set_attenuator(value[2]);
      }
    }
  } else if (handle == ble_attr_rx_cccdhandle) {
    if (length == 2) {
      if (value[0] == 0x01) {
        logger->printf(LOG_RTT,"notify enable\n");
        is_notification_enabled = true;
      } else if (value[0] == 0x00) {
        logger->printf(LOG_RTT,"notify disable\n");
        is_notification_enabled = false;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
ble_gap_conn_evt_handler(uint16_t conn_handle, uint16_t state)
{
  peripheral_conn_handle = conn_handle;
  if (state == BLE_GATT_STATE_CONNECTED) {
    is_connected = true;
    pin->output(LED0, PIN_OUTPUT_HIGH);
    pin->output(LED1, PIN_OUTPUT_HIGH);
    pin->output(LED2, PIN_OUTPUT_HIGH);
  } else if (state == BLE_GATT_STATE_DISCONNECTED) {
    is_connected = false;
    pin->output(LED0, PIN_OUTPUT_LOW);
    pin->output(LED1, PIN_OUTPUT_LOW);
    pin->output(LED2, PIN_OUTPUT_LOW);
  }
}
/*---------------------------------------------------------------------------*/
static void
adv_sendout_callback(void)
{
  adv_event_flag  =true;
}
/*---------------------------------------------------------------------------*/
const static BleAdvertiseCallback adv_callback = {
  .onSendout = adv_sendout_callback
};
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
  Process *process = OSProcess();
  logger = OSLogger();
  ble_manager = CKBleManager();
  adv_builder = CKAdvDataBuilder();
  device = OSDevice();
  pin = HWPin();
  pin->setup(LED0, PIN_OUTPUT);
  pin->setup(LED1, PIN_OUTPUT);
  pin->setup(LED2, PIN_OUTPUT);
  pin->output(LED0, PIN_OUTPUT_LOW);
  pin->output(LED1, PIN_OUTPUT_LOW);
  pin->output(LED2, PIN_OUTPUT_LOW);

  pin->setup(RADIO_ATT_1dbm, PIN_OUTPUT);
  pin->setup(RADIO_ATT_2dbm, PIN_OUTPUT);
  pin->setup(RADIO_ATT_4dbm, PIN_OUTPUT);
  pin->setup(RADIO_ATT_8dbm, PIN_OUTPUT);
  pin->setup(RADIO_ATT_16dbm, PIN_OUTPUT);

  pin->output(RADIO_ATT_1dbm, PIN_OUTPUT_HIGH);
  pin->output(RADIO_ATT_2dbm, PIN_OUTPUT_HIGH);
  pin->output(RADIO_ATT_4dbm, PIN_OUTPUT_HIGH);
  pin->output(RADIO_ATT_8dbm, PIN_OUTPUT_HIGH);
  pin->output(RADIO_ATT_16dbm, PIN_OUTPUT_HIGH);

  beacon_major[0] = 0x17;
  beacon_major[1] = 0x74;

  beacon_minor[0] = 0x00;
  beacon_minor[1] = 0x09;

  android_battery_packetes[0] = 0xFF;
  android_battery_packetes[1] = 0x10;
  android_battery_packetes[2] = 0x53;  //battery

  ios_battery_packetes[0] = beacon_major[0];
  ios_battery_packetes[1] = beacon_major[1];
  ios_battery_packetes[2] = beacon_minor[0];  //battery
  ios_battery_packetes[3] = beacon_minor[1];  //battery
  ios_battery_packetes[4] = 0x53;  //battery

  // setup the ibeacon data
  adv_builder->init(&advdata);
  adv_builder->addManufacturerData(&advdata, APP_COMPANY_IDENTIFIER, manufacturer_ibeacon_data, APP_BEACON_INFO_LENGTH);

  adv_builder->initScanRsp(&scan_rsp_advdata);
  adv_builder->addCompleteLocalName(&scan_rsp_advdata, device_name, strlen(device_name));

  uint16_t android_service_uuid = 0xFF10;
  adv_builder->addServiceData(&scan_rsp_advdata, android_service_uuid, &android_battery_packetes[2], 1);

  uint16_t ios_service_uuid = 0x1774;
  adv_builder->addServiceData(&scan_rsp_advdata, ios_service_uuid, &ios_battery_packetes[2], 3);

  errcode = ble_manager->setAdvertisementData(&advdata, &scan_rsp_advdata);
  if (errcode != ENONE) {
    logger->printf(LOG_RTT, "[app] setAdvertisementData error %d\n", errcode);
    return 0;
  }

  errcode = ble_manager->addService(&g_ble_gatt_nus_service);
  if (errcode != ENONE) {
    logger->printf(LOG_RTT,"[app] addService error %d\n", errcode);
    return 0;
  }

  errcode = ble_manager->startAdvertising(ADV_INTERVAL_LEVEL_0, &adv_callback, &callback);
  if (errcode != ENONE) {
    logger->printf(LOG_RTT,"[app] startAdvertising error %d\n", errcode);
    return 0;
  }

  while (1) {
    // Power saving
    process->waitForEvent();

    if (adv_event_flag) {
      adv_event_flag = false;
      pin->output(LED0, PIN_OUTPUT_HIGH);
      process->delay(10);
      pin->output(LED0, PIN_OUTPUT_LOW);
    }
  }
  return 0;
}
