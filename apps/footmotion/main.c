#include "alfabase.h"

/* Data ready from motion sensor. */
#define MOTIONFUSION_ACCEL                (0x01)
#define MOTIONFUSION_GYRO                 (0x02)
#define MOTIONFUSION_COMPASS              (0x04)
#define MOTIONFUSION_QUAT                 (0x08)
#define MOTIONFUSION_EULER                (0x10)
#define MOTIONFUSION_HEADING              (0x20)
#define MOTIONFUSION_LINEAR_ACCEL         (0x40)
#define MOTIONFUSION_GRAVITY_VECTOR       (0x80)

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
} linear_accel_data_t;

typedef struct {
	float value[3];
	uint32_t timestamp;
} gravity_vector_t;

typedef struct {
	float value;
	int32_t data;
	uint32_t timestamp;
} heading_data_t;

typedef struct {
  uint8_t type;
} motion_data_event_t;

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
/* AlfaOne Sensor Acceleration UUID {79430003-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfaone_sensor_accel_chr_uuid = BLE_UUID16_INIT(0x0003);
/* AlfaOne Sensor Gyro UUID {79430004-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfaone_sensor_gyro_chr_uuid = BLE_UUID16_INIT(0x0004);
/* AlfaOne Sensor Compass UUID {79430005-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfaone_sensor_compass_chr_uuid = BLE_UUID16_INIT(0x0005);
/* AlfaOne Sensor Quaternion UUID {79430006-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfaone_sensor_quat_chr_uuid = BLE_UUID16_INIT(0x0006);
/* AlfaOne Sensor Euler UUID {79430007-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfaone_sensor_euler_chr_uuid = BLE_UUID16_INIT(0x0007);
/* AlfaOne Sensor Lineal Acceleration UUID {79430008-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfaone_sensor_linear_accel_chr_uuid = BLE_UUID16_INIT(0x0008);
/* AlfaOne Sensor Gravity Vector UUID {79430009-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfaone_sensor_gravity_vector_chr_uuid = BLE_UUID16_INIT(0x0009);
/* AlfaOne Sensor Heading UUID {79430009-F6C2-09A3-E9F9-128ABCA31297} */
const BleUuid16 gatt_alfaone_sensor_heading_chr_uuid = BLE_UUID16_INIT(0x000A);
/*---------------------------------------------------------------------------*/
const static char *device_name = "AlfaOne";
static bool timer_flag = false;
static uint8_t m_mac_address[6];
static uint8_t service_data_packetes[8];
static uint16_t peripheral_conn_handle;
static bool is_connected = false;
/*---------------------------------------------------------------------------*/
static Logger *logger;
static AdvDataBuilder *adv_builder;
static BleManager *ble_manager;
static AdvData advdata;
static AdvData scan_rsp_advdata;
static HWDriver *mpudmp;
static HWDriver *ieefp4;
static Timer *sample_timer;
/*---------------------------------------------------------------------------*/
static uint16_t ble_srv_alfaone_sensor_handle;
static uint16_t ble_attr_ieefp4_handle;
static uint16_t ble_attr_ieefp4_cccd_handle;
static uint16_t ble_attr_accel_handle;
static uint16_t ble_attr_accel_cccd_handle;
static uint16_t ble_attr_gyro_handle;
static uint16_t ble_attr_gyro_cccd_handle;
static uint16_t ble_attr_compass_handle;
static uint16_t ble_attr_compass_cccd_handle;
static uint16_t ble_attr_quat_handle;
static uint16_t ble_attr_quat_cccd_handle;
static uint16_t ble_attr_euler_handle;
static uint16_t ble_attr_euler_cccd_handle;
static uint16_t ble_attr_linear_accel_handle;
static uint16_t ble_attr_linear_accel_cccd_handle;
static uint16_t ble_attr_gravity_vector_handle;
static uint16_t ble_attr_gravity_vector_cccd_handle;
static uint16_t ble_attr_heading_handle;
static uint16_t ble_attr_heading_cccd_handle;
/*---------------------------------------------------------------------------*/
static uint8_t ieefp4_tx_buffer[12];
static uint8_t accel_tx_buffer[16];
static uint8_t gyro_tx_buffer[16];
static uint8_t compass_tx_buffer[16];
static uint8_t quat_tx_buffer[20];
static uint8_t euler_tx_buffer[16];
static uint8_t linear_accel_tx_buffer[16];
static uint8_t gravity_vector_tx_buffer[16];
static uint8_t heading_tx_buffer[8];
/*---------------------------------------------------------------------------*/
static bool is_ieefp4_notify_enabled = false;
static bool is_accel_notify_enabled = false;
static bool is_gyro_notify_enabled = false;
static bool is_compass_notify_enabled = false;
static bool is_quat_notify_enabled = false;
static bool is_euler_notify_enabled = false;
static bool is_linear_accel_notify_enabled = false;
static bool is_gravity_vector_notify_enabled = false;
static bool is_heading_notify_enabled = false;
/*---------------------------------------------------------------------------*/
static mems_data_t accel_data;
static mems_data_t gyro_data;
static mems_data_t compass_data;
static quat_data_t quat_data;
static mems_data_t euler_data;
static linear_accel_data_t linear_accel_data;
static gravity_vector_t gravity_vector;
static heading_data_t heading_data;
static ieefp4_data_t ieefp4_data_inst;
/*---------------------------------------------------------------------------*/
static BleGattService g_ble_gatt_alfa_ibeacon_service = {
  .type = BLE_GATT_SERVICE_TYPE_PRIMARY,
  .uuid = &gatt_alfaone_sensor_service_uuid,
  .handle = &ble_srv_alfaone_sensor_handle,
  .characteristic_count = 9,
  .characteristics = (BleGattCharacteristic[]) { {
    .uuid = &gatt_alfaone_sensor_ieefp4_chr_uuid,
    .value_handle = &ble_attr_ieefp4_handle,
    .cccd_handle = &ble_attr_ieefp4_cccd_handle,
    .props = BLE_GATT_CHR_PROPS_NOTIFY,
    .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
    .init_value = &ieefp4_tx_buffer[0],
    .init_value_len = 12,
  }, {
    .uuid = &gatt_alfaone_sensor_accel_chr_uuid,
    .value_handle = &ble_attr_accel_handle,
    .cccd_handle = &ble_attr_accel_cccd_handle,
    .props = BLE_GATT_CHR_PROPS_NOTIFY,
    .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
    .init_value = &accel_tx_buffer[0],
    .init_value_len = 16,
  }, {
    .uuid = &gatt_alfaone_sensor_gyro_chr_uuid,
    .value_handle = &ble_attr_gyro_handle,
    .cccd_handle = &ble_attr_gyro_cccd_handle,
    .props = BLE_GATT_CHR_PROPS_NOTIFY,
    .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
    .init_value = &gyro_tx_buffer[0],
    .init_value_len = 16,
  }, {
    .uuid = &gatt_alfaone_sensor_compass_chr_uuid,
    .value_handle = &ble_attr_compass_handle,
    .cccd_handle = &ble_attr_compass_cccd_handle,
    .props = BLE_GATT_CHR_PROPS_NOTIFY,
    .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
    .init_value = &compass_tx_buffer[0],
    .init_value_len = 16,
  }, {
    .uuid = &gatt_alfaone_sensor_quat_chr_uuid,
    .value_handle = &ble_attr_quat_handle,
    .cccd_handle = &ble_attr_quat_cccd_handle,
    .props = BLE_GATT_CHR_PROPS_NOTIFY,
    .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
    .init_value = &quat_tx_buffer[0],
    .init_value_len = 20,
  }, {
    .uuid = &gatt_alfaone_sensor_euler_chr_uuid,
    .value_handle = &ble_attr_euler_handle,
    .cccd_handle = &ble_attr_euler_cccd_handle,
    .props = BLE_GATT_CHR_PROPS_NOTIFY,
    .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
    .init_value = &euler_tx_buffer[0],
    .init_value_len = 16,
  }, {
    .uuid = &gatt_alfaone_sensor_linear_accel_chr_uuid,
    .value_handle = &ble_attr_linear_accel_handle,
    .cccd_handle = &ble_attr_linear_accel_cccd_handle,
    .props = BLE_GATT_CHR_PROPS_NOTIFY,
    .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
    .init_value = &linear_accel_tx_buffer[0],
    .init_value_len = 16,
  }, {
    .uuid = &gatt_alfaone_sensor_gravity_vector_chr_uuid,
    .value_handle = &ble_attr_gravity_vector_handle,
    .cccd_handle = &ble_attr_gravity_vector_cccd_handle,
    .props = BLE_GATT_CHR_PROPS_NOTIFY,
    .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
    .init_value = &gravity_vector_tx_buffer[0],
    .init_value_len = 16,
  }, {
    .uuid = &gatt_alfaone_sensor_heading_chr_uuid,
    .value_handle = &ble_attr_heading_handle,
    .cccd_handle = &ble_attr_heading_cccd_handle,
    .props = BLE_GATT_CHR_PROPS_NOTIFY,
    .permission = BLE_GATT_CHR_PERMISSION_READ | BLE_GATT_CHR_PERMISSION_WRITE,
    .init_value = &heading_tx_buffer[0],
    .init_value_len = 8,
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
mpu9250_dmp_data_update(void *params)
{
	motion_data_event_t *event = (motion_data_event_t *)params;
	uint8_t type = event->type;
	// logger->printf(LOG_RTT,"[app] dmp data update %d\n", type);

	if (event->type & MOTIONFUSION_ACCEL) {
		mpudmp->read(&accel_data, sizeof(mems_data_t), 0);
		memcpy(&accel_tx_buffer[0], &accel_data.data[0], sizeof(int32_t));
		memcpy(&accel_tx_buffer[4], &accel_data.data[1], sizeof(int32_t));
		memcpy(&accel_tx_buffer[8], &accel_data.data[2], sizeof(int32_t));
		memcpy(&accel_tx_buffer[12], &accel_data.timestamp, sizeof(uint32_t));
	}
	if (event->type & MOTIONFUSION_GYRO) {
		mpudmp->read(&gyro_data, sizeof(mems_data_t), 1);
		memcpy(&gyro_tx_buffer[0], &gyro_data.data[0], sizeof(int32_t));
		memcpy(&gyro_tx_buffer[4], &gyro_data.data[1], sizeof(int32_t));
		memcpy(&gyro_tx_buffer[8], &gyro_data.data[2], sizeof(int32_t));
		memcpy(&gyro_tx_buffer[12], &gyro_data.timestamp, sizeof(uint32_t));
	}
	if (event->type & MOTIONFUSION_COMPASS) {
		mpudmp->read(&compass_data, sizeof(mems_data_t), 2);
		memcpy(&compass_tx_buffer[0], &compass_data.data[0], sizeof(int32_t));
		memcpy(&compass_tx_buffer[4], &compass_data.data[1], sizeof(int32_t));
		memcpy(&compass_tx_buffer[8], &compass_data.data[2], sizeof(int32_t));
		memcpy(&compass_tx_buffer[12], &compass_data.timestamp, sizeof(uint32_t));
	}
	if (event->type & MOTIONFUSION_QUAT) {
		mpudmp->read(&quat_data, sizeof(quat_data_t), 3);
		memcpy(&quat_tx_buffer[0], &quat_data.data[0], sizeof(int32_t));
		memcpy(&quat_tx_buffer[4], &quat_data.data[1], sizeof(int32_t));
		memcpy(&quat_tx_buffer[8], &quat_data.data[2], sizeof(int32_t));
		memcpy(&quat_tx_buffer[12], &quat_data.data[3], sizeof(int32_t));
		memcpy(&quat_tx_buffer[16], &quat_data.timestamp, sizeof(uint32_t));
	}
	if (event->type & MOTIONFUSION_EULER) {
		mpudmp->read(&euler_data, sizeof(mems_data_t), 4);
		memcpy(&euler_tx_buffer[0], &euler_data.data[0], sizeof(int32_t));
		memcpy(&euler_tx_buffer[4], &euler_data.data[1], sizeof(int32_t));
		memcpy(&euler_tx_buffer[8], &euler_data.data[2], sizeof(int32_t));
		memcpy(&euler_tx_buffer[12], &euler_data.timestamp, sizeof(uint32_t));
	}
	if (event->type & MOTIONFUSION_LINEAR_ACCEL) {
		mpudmp->read(&linear_accel_data, sizeof(linear_accel_data_t), 5);
		memcpy(&linear_accel_tx_buffer[0], &linear_accel_data.value[0], sizeof(float));
		memcpy(&linear_accel_tx_buffer[4], &linear_accel_data.value[1], sizeof(float));
		memcpy(&linear_accel_tx_buffer[8], &linear_accel_data.value[2], sizeof(float));
		memcpy(&linear_accel_tx_buffer[12], &linear_accel_data.timestamp, sizeof(uint32_t));
	}
	if (event->type & MOTIONFUSION_GRAVITY_VECTOR) {
		mpudmp->read(&gravity_vector, sizeof(gravity_vector_t), 6);
		memcpy(&gravity_vector_tx_buffer[0], &gravity_vector.value[0], sizeof(float));
		memcpy(&gravity_vector_tx_buffer[4], &gravity_vector.value[1], sizeof(float));
		memcpy(&gravity_vector_tx_buffer[8], &gravity_vector.value[2], sizeof(float));
		memcpy(&gravity_vector_tx_buffer[12], &gravity_vector.timestamp, sizeof(uint32_t));
	}
	if (event->type & MOTIONFUSION_HEADING) {
		mpudmp->read(&heading_data, sizeof(heading_data_t), 7);
		memcpy(&heading_tx_buffer[0], &heading_data.value, sizeof(float));
		memcpy(&heading_tx_buffer[4], &heading_data.timestamp, sizeof(uint32_t));
	}
}
/*---------------------------------------------------------------------------*/
static void
ble_gatt_chr_write_req(uint16_t conn_handle, uint16_t handle, uint8_t *value, uint16_t length)
{
  // data from peer
  if (length == 2) {
    if (value[0] == 0x01) {
      if (handle == ble_attr_ieefp4_cccd_handle)
        is_ieefp4_notify_enabled = true;
      else if (handle == ble_attr_accel_cccd_handle)
        is_accel_notify_enabled = true;
      else if (handle == ble_attr_gyro_cccd_handle)
        is_gyro_notify_enabled = true;
			else if (handle == ble_attr_compass_cccd_handle)
				is_compass_notify_enabled = true;
			else if (handle == ble_attr_quat_cccd_handle)
			  is_quat_notify_enabled = true;
			else if (handle == ble_attr_euler_cccd_handle)
			  is_euler_notify_enabled = true;
			else if (handle == ble_attr_linear_accel_cccd_handle)
			  is_linear_accel_notify_enabled = true;
			else if (handle == ble_attr_gravity_vector_cccd_handle)
			  is_gravity_vector_notify_enabled = true;
			else if (handle == ble_attr_heading_cccd_handle)
			  is_heading_notify_enabled = true;

    } else if (value[0] == 0x00) {
      if (handle == ble_attr_ieefp4_cccd_handle)
        is_ieefp4_notify_enabled = false;
      else if (handle == ble_attr_accel_cccd_handle)
        is_accel_notify_enabled = false;
      else if (handle == ble_attr_gyro_cccd_handle)
        is_gyro_notify_enabled = false;
			else if (handle == ble_attr_compass_cccd_handle)
				is_compass_notify_enabled = false;
			else if (handle == ble_attr_quat_cccd_handle)
			  is_quat_notify_enabled = false;
			else if (handle == ble_attr_euler_cccd_handle)
			  is_euler_notify_enabled = false;
			else if (handle == ble_attr_linear_accel_cccd_handle)
			  is_linear_accel_notify_enabled = false;
			else if (handle == ble_attr_gravity_vector_cccd_handle)
			  is_gravity_vector_notify_enabled = false;
			else if (handle == ble_attr_heading_cccd_handle)
			  is_heading_notify_enabled = false;
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
		mpudmp->subscribe(NULL, 0, mpu9250_dmp_data_update);
    ieefp4->open(NULL);
    sample_timer->start(0, 100, timer_event_handler);
    is_connected = true;
  } else if (state == BLE_GATT_STATE_DISCONNECTED) {
    sample_timer->stop(0);
    mpudmp->close(NULL);
    ieefp4->close(NULL);
    is_ieefp4_notify_enabled = false;
    is_accel_notify_enabled = false;
    is_gyro_notify_enabled = false;
		is_compass_notify_enabled = false;
		is_quat_notify_enabled = false;
		is_euler_notify_enabled = false;
		is_linear_accel_notify_enabled = false;
		is_gravity_vector_notify_enabled = false;
		is_heading_notify_enabled = false;
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
				// read the foot pressure
        ieefp4->read(&ieefp4_data_inst, sizeof(ieefp4_data_t), 0);
				memcpy(&ieefp4_tx_buffer[0], &ieefp4_data_inst.heel, sizeof(int16_t));
				memcpy(&ieefp4_tx_buffer[2], &ieefp4_data_inst.outer_ball, sizeof(int16_t));
				memcpy(&ieefp4_tx_buffer[4], &ieefp4_data_inst.inner_ball, sizeof(int16_t));
				memcpy(&ieefp4_tx_buffer[6], &ieefp4_data_inst.thumb, sizeof(int16_t));
				memcpy(&ieefp4_tx_buffer[8], &ieefp4_data_inst.timestamp, sizeof(uint32_t));

				// notify
        if (is_ieefp4_notify_enabled)
          ble_manager->notifyCharacteristic(peripheral_conn_handle, ble_attr_ieefp4_handle, ieefp4_tx_buffer, 12);
        if (is_accel_notify_enabled)
          ble_manager->notifyCharacteristic(peripheral_conn_handle, ble_attr_accel_handle, accel_tx_buffer, 16);
        if (is_gyro_notify_enabled)
          ble_manager->notifyCharacteristic(peripheral_conn_handle, ble_attr_gyro_handle, gyro_tx_buffer, 16);
				if (is_compass_notify_enabled)
					ble_manager->notifyCharacteristic(peripheral_conn_handle, ble_attr_compass_handle, compass_tx_buffer, 16);
				if (is_quat_notify_enabled)
					ble_manager->notifyCharacteristic(peripheral_conn_handle, ble_attr_quat_handle, quat_tx_buffer, 20);
				if (is_euler_notify_enabled)
					ble_manager->notifyCharacteristic(peripheral_conn_handle, ble_attr_euler_handle, euler_tx_buffer, 16);
				if (is_linear_accel_notify_enabled)
					ble_manager->notifyCharacteristic(peripheral_conn_handle, ble_attr_linear_accel_handle, linear_accel_tx_buffer, 16);
				if (is_gravity_vector_notify_enabled)
					ble_manager->notifyCharacteristic(peripheral_conn_handle, ble_attr_gravity_vector_handle, gravity_vector_tx_buffer, 16);
				if (is_heading_notify_enabled)
					ble_manager->notifyCharacteristic(peripheral_conn_handle, ble_attr_heading_handle, heading_tx_buffer, 8);
      }
    }
  }
  return 0;
}
