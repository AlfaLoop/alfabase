#include "alfabase.h"

typedef struct {
	float value[3];
	int32_t data[3];
	uint32_t timestamp;
} mems_data_t;

typedef struct {
	float value[4];
	int32_t data[4];
	uint32_t timestamp;
} quat_data_t;

typedef struct {
	float value[3];
	uint32_t timestamp;
} vector_data_t;

typedef struct {
	float value;
	int32_t data;
	uint32_t timestamp;
} heading_data_t;

typedef struct {
	int16_t  heel;
	int16_t  outer_ball;
	int16_t  inner_ball;
	int16_t  thumb;
	uint32_t timestamp;
} ieefp4_data_t;

// AlfaOne Sensor Profile
/* {79430001-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid128 gatt_alfaone_sensor_service_uuid =
    BLE_UUID128_INIT(0x97, 0x12, 0xa3, 0xbc, 0x8a, 0x12, 0xF9, 0xE9,
                     0xa3, 0x09, 0xc2, 0xF6, 0x01, 0x00, 0x43, 0x79);
/* AlfaOne Sensor IEEFP4 UUID {79430002-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfaone_sensor_ieefp4_chr_uuid = BLE_UUID16_INIT(0x0002);
/* AlfaOne Sensor Accel UUID {79430003-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfaone_sensor_accel_chr_uuid = BLE_UUID16_INIT(0x0003);
/* AlfaOne Sensor Gyro UUID {79430004-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfaone_sensor_gyro_chr_uuid = BLE_UUID16_INIT(0x0004);

static bool timer_flag = false;
const static char *device_name = "AlfaOne";

static Logger *logger;
static AdvDataBuilder *adv_builder;
static BleManager *ble_manager;
static AdvData advdata;
static AdvData scan_rsp_advdata;
static HWDriver *mpudmp;
static HWDriver *ieefp4;
static Timer *sample_timer;

static uint16_t ble_srv_alfaone_sensor_handle;
static uint16_t ble_attr_alfaone_ieefp4_handle;
static uint16_t ble_attr_alfaone_ieefp4_cccd_handle;
static uint16_t ble_attr_alfaone_accel_handle;
static uint16_t ble_attr_alfaone_accel_cccd_handle;
static uint16_t ble_attr_alfaone_gyro_handle;
static uint16_t ble_attr_alfaone_gyro_cccd_handle;

static uint8_t ieefp4_tx_buffer[12];
static uint8_t accel_tx_buffer[16];
static uint8_t gyro_tx_buffer[16];

static uint8_t m_mac_address[6];
static uint8_t service_data_packetes[8];
static uint16_t peripheral_conn_handle;
static bool is_connected = false;
static bool is_ieefp4_notification_enabled = false;
static bool is_accel_notification_enabled = false;
static bool is_gyro_notification_enabled = false;
static mems_data_t accel_data;
static mems_data_t gyro_data;
static ieefp4_data_t ieefp4_data_inst;



static BleGattService g_ble_gatt_alfa_ibeacon_service = {
  .type = BLE_GATT_SERVICE_TYPE_PRIMARY,
  .uuid = &gatt_alfaone_sensor_service_uuid,
  .handle = &ble_srv_alfaone_sensor_handle,
  .characteristic_count = 3,
  .characteristics = (BleGattCharacteristic[]) { {
    .uuid = &gatt_alfaone_sensor_ieefp4_chr_uuid,
    .value_handle = &ble_attr_alfaone_ieefp4_handle,
    .cccd_handle = &ble_attr_alfaone_ieefp4_cccd_handle,
    .props = BLE_GATT_CHR_PROPS_NOTIFY,
    .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
    .init_value = &ieefp4_tx_buffer[0],
    .init_value_len = 12,
  }, {
    .uuid = &gatt_alfaone_sensor_accel_chr_uuid,
    .value_handle = &ble_attr_alfaone_accel_handle,
    .cccd_handle = &ble_attr_alfaone_accel_cccd_handle,
    .props = BLE_GATT_CHR_PROPS_NOTIFY,
    .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
    .init_value = &accel_tx_buffer[0],
    .init_value_len = 16,
  }, {
    .uuid = &gatt_alfaone_sensor_gyro_chr_uuid,
    .value_handle = &ble_attr_alfaone_gyro_handle,
    .cccd_handle = &ble_attr_alfaone_gyro_cccd_handle,
    .props = BLE_GATT_CHR_PROPS_NOTIFY,
    .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
    .init_value = &gyro_tx_buffer[0],
    .init_value_len = 16,
  }, {
    0, /* End: No more characteristics in this service */
  }}
};
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static void
timer_event_handler(void)
{
  timer_flag = true;
}
/*---------------------------------------------------------------------------*/
static void
ble_gatt_chr_write_req(uint16_t conn_handle, uint16_t handle, uint8_t *value, uint16_t length)
{
  // data from peer
  if (length == 2) {
    if (value[0] == 0x01) {
      if (handle == ble_attr_alfaone_ieefp4_cccd_handle)
        is_ieefp4_notification_enabled = true;
      else if (handle == ble_attr_alfaone_accel_cccd_handle)
        is_accel_notification_enabled = true;
      else if (handle == ble_attr_alfaone_gyro_cccd_handle)
        is_gyro_notification_enabled = true;
    } else if (value[0] == 0x00) {
      if (handle == ble_attr_alfaone_ieefp4_cccd_handle)
        is_ieefp4_notification_enabled = false;
      else if (handle == ble_attr_alfaone_accel_cccd_handle)
        is_accel_notification_enabled = false;
      else if (handle == ble_attr_alfaone_gyro_cccd_handle)
        is_gyro_notification_enabled = false;
    }
  }

}
/*---------------------------------------------------------------------------*/
static void
ble_gap_conn_evt_handler(uint16_t conn_handle, uint16_t state)
{
  peripheral_conn_handle = conn_handle;
  if (state == BLE_GATT_STATE_CONNECTED) {
    mpudmp->open(NULL);
    ieefp4->open(NULL);
    sample_timer->start(0, 100, timer_event_handler);
    is_connected = true;
  } else if (state == BLE_GATT_STATE_DISCONNECTED) {
    sample_timer->stop(0);
    mpudmp->close(NULL);
    ieefp4->close(NULL);
    is_ieefp4_notification_enabled = false;
    is_accel_notification_enabled = false;
    is_gyro_notification_enabled = false;
    is_connected = false;
    timer_flag = false;
  }
}
/*---------------------------------------------------------------------------*/
const static BleGattServerCallback callback = {
  .onCharacteristicWriteRequest = ble_gatt_chr_write_req,
  .onCharacteristicReadRequest = NULL,
  .onConnectionStateChanged = ble_gap_conn_evt_handler
};
/*---------------------------------------------------------------------------*/
static void
setup_advertisement(void)
{
  uint16_t service_uuid = 0xA55A;

  // setup the data
  adv_builder->init(&advdata);

  // Add the completed local name
  adv_builder->addCompleteLocalName(&advdata, device_name, strlen(device_name));

  adv_builder->addService16bitUUID(&advdata, service_uuid);

  // Add the service uuid and data
  service_data_packetes[0] = 0x09;  // device type
  service_data_packetes[1] = 0x64;  // battery level
  service_data_packetes[2] = m_mac_address[5];
  service_data_packetes[3] = m_mac_address[4];
  service_data_packetes[4] = m_mac_address[3];
  service_data_packetes[5] = m_mac_address[2];
  service_data_packetes[6] = m_mac_address[1];
  service_data_packetes[7] = m_mac_address[0];
  adv_builder->addServiceData(&advdata, service_uuid, &service_data_packetes[0], 8);

  // setup the advertisement
  ble_manager->setAdvertisementData(&advdata, NULL);
}
/*---------------------------------------------------------------------------*/
int main(void)
{
  int errcode;
  Process *process = OSProcess();
  logger = OSLogger();

  // Get the timer instance 0
  sample_timer = OSTimer();
  ble_manager = CKBleManager();
  adv_builder = CKAdvDataBuilder();

  mpudmp = HWPipe("mpu9250_dmp");
  ieefp4 = HWPipe("ieefp4");

  // Get the mac address
  ble_manager->getMacAddress(m_mac_address);
  ble_manager->setDeviceName(device_name, strlen(device_name));

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
    if (timer_flag) {
      timer_flag = false;
      if (is_connected) {
        mpudmp->read(&accel_data, sizeof(mems_data_t), 0);
        mpudmp->read(&gyro_data, sizeof(mems_data_t), 1);
        ieefp4->read(&ieefp4_data_inst, sizeof(ieefp4_data_t), 0);

        // copy the ieefp4 data
        ieefp4_tx_buffer[0] = ieefp4_data_inst.heel & 0x00FF;
        ieefp4_tx_buffer[1] = ieefp4_data_inst.heel >> 8;
        ieefp4_tx_buffer[2] = ieefp4_data_inst.outer_ball & 0x00FF;
        ieefp4_tx_buffer[3] = ieefp4_data_inst.outer_ball >> 8;
        ieefp4_tx_buffer[4] = ieefp4_data_inst.inner_ball & 0x00FF;
        ieefp4_tx_buffer[5] = ieefp4_data_inst.inner_ball >> 8;
        ieefp4_tx_buffer[6] = ieefp4_data_inst.thumb & 0x00FF;
        ieefp4_tx_buffer[7] = ieefp4_data_inst.thumb >> 8;
        ieefp4_tx_buffer[8] = ieefp4_data_inst.timestamp & 0x000000FF;
        ieefp4_tx_buffer[9] = (ieefp4_data_inst.timestamp & 0x0000FF00) >> 8;
        ieefp4_tx_buffer[10] = (ieefp4_data_inst.timestamp & 0x00FF0000) >> 16;
        ieefp4_tx_buffer[11] = (ieefp4_data_inst.timestamp & 0xFF000000) >> 24;

        accel_tx_buffer[0] = accel_data.data[0] & 0x000000FF;
        accel_tx_buffer[1] = (accel_data.data[0] & 0x0000FF00) >> 8;
        accel_tx_buffer[2] = (accel_data.data[0] & 0x00FF0000) >> 16;
        accel_tx_buffer[3] = (accel_data.data[0] & 0xFF000000) >> 24;
        accel_tx_buffer[4] = accel_data.data[1] & 0x000000FF;
        accel_tx_buffer[5] = (accel_data.data[1] & 0x0000FF00) >> 8;
        accel_tx_buffer[6] = (accel_data.data[1] & 0x00FF0000) >> 16;
        accel_tx_buffer[7] = (accel_data.data[1] & 0xFF000000) >> 24;
        accel_tx_buffer[8] = accel_data.data[2] & 0x000000FF;
        accel_tx_buffer[9] = (accel_data.data[2] & 0x0000FF00) >> 8;
        accel_tx_buffer[10] = (accel_data.data[2] & 0x00FF0000) >> 16;
        accel_tx_buffer[11] = (accel_data.data[2] & 0xFF000000) >> 24;
        accel_tx_buffer[12] = accel_data.timestamp & 0x000000FF;
        accel_tx_buffer[13] = (accel_data.timestamp & 0x0000FF00) >> 8;
        accel_tx_buffer[14] = (accel_data.timestamp & 0x00FF0000) >> 16;
        accel_tx_buffer[15] = (accel_data.timestamp & 0xFF000000) >> 24;

        gyro_tx_buffer[0] = gyro_data.data[0] & 0x000000FF;
        gyro_tx_buffer[1] = (gyro_data.data[0] & 0x0000FF00) >> 8;
        gyro_tx_buffer[2] = (gyro_data.data[0] & 0x00FF0000) >> 16;
        gyro_tx_buffer[3] = (gyro_data.data[0] & 0xFF000000) >> 24;
        gyro_tx_buffer[4] = gyro_data.data[1] & 0x000000FF;
        gyro_tx_buffer[5] = (gyro_data.data[1] & 0x0000FF00) >> 8;
        gyro_tx_buffer[6] = (gyro_data.data[1] & 0x00FF0000) >> 16;
        gyro_tx_buffer[7] = (gyro_data.data[1] & 0xFF000000) >> 24;
        gyro_tx_buffer[8] = gyro_data.data[2] & 0x000000FF;
        gyro_tx_buffer[9] = (gyro_data.data[2] & 0x0000FF00) >> 8;
        gyro_tx_buffer[10] = (gyro_data.data[2] & 0x00FF0000) >> 16;
        gyro_tx_buffer[11] = (gyro_data.data[2] & 0xFF000000) >> 24;
        gyro_tx_buffer[12] = gyro_data.timestamp & 0x000000FF;
        gyro_tx_buffer[13] = (gyro_data.timestamp & 0x0000FF00) >> 8;
        gyro_tx_buffer[14] = (gyro_data.timestamp & 0x00FF0000) >> 16;
        gyro_tx_buffer[15] = (gyro_data.timestamp & 0xFF000000) >> 24;

        if (is_ieefp4_notification_enabled)
          ble_manager->notifyCharacteristic(peripheral_conn_handle, ble_attr_alfaone_ieefp4_handle, ieefp4_tx_buffer, 12);
        if (is_accel_notification_enabled)
          ble_manager->notifyCharacteristic(peripheral_conn_handle, ble_attr_alfaone_accel_handle, accel_tx_buffer, 16);
        if (is_gyro_notification_enabled)
          ble_manager->notifyCharacteristic(peripheral_conn_handle, ble_attr_alfaone_gyro_handle, gyro_tx_buffer, 16);
      }
    }
  }
  return 0;
}
