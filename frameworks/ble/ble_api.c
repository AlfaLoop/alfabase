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
#include "loader/symtab.h"
#include "frameworks/app_lifecycle.h"
#include "frameworks/app_eventpool.h"
#include "frameworks/ble/ble_api.h"
#include "frameworks/ble/ble_gatt_api.h"
#include "frameworks/ble/ble_uuid_api.h"
#include "nest/nest.h"
#include "errno.h"
#include "FreeRTOS.h"
#include "task.h"
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
#define ADV_LEN_FIELD_SIZE              1UL
//  Advertising Data and Scan Response format contains 1 octet for the AD type
#define ADV_TYPE_FIELD_SIZE             1UL
#define ADV_DATA_OFFSET                 (ADV_LEN_FIELD_SIZE + ADV_TYPE_FIELD_SIZE)
//  Size of the Company Identifier Code, which is a part of the Manufacturer Specific Data AD type.
#define ADV_TYPE_MANUF_SPEC_DATA_ID_SIZE    2UL

// Manufacturer Specific Data.
#define ADV_TYPE_MANUFACTURER_SPECIFIC_DATA          0xFF

extern TaskHandle_t g_contiki_thread;

static BleGattServerCallback m_ble_gatt_server_callback;
static BleScanCallback m_ble_scanner_callback;
static BleAdvertiseCallback m_ble_adv_callback;

// ble scan api
static bool m_ble_scan_started = false;
static uint16_t m_ble_scan_interval_level = SCAN_INTERVAL_LEVEL_1;

// ble adv api
static bool m_ble_adv_started = false;
static uint16_t m_ble_adv_interval_level = ADV_INTERVAL_LEVEL_1;
static uint8_t m_ble_adv_type = 0;
static bool m_ble_pheripheral_connected = false;

// ble tx power config
static int8_t m_tx_power = PLATFORM_DEVICE_TX_POWER;

/*---------------------------------------------------------------------------*/
static void
ble_scan_api_irq_hooker(void *ptr)
{
  app_irq_ble_scan_event_t *p_event = (app_irq_ble_scan_event_t *) ptr;
  static ScanRecord record;

  // PRINTF("[ble api] scan address %02x:%02x:%02x:%02x:%02x:%02x\n", p_event->record.device.address[0],
  //     p_event->record.device.address[1], p_event->record.device.address[2], p_event->record.device.address[3],
  //     p_event->record.device.address[4], p_event->record.device.address[5]);
  memset(&record, 0x00, sizeof(ScanRecord));
  switch (p_event->type) {
    case NEST_SCANNER_ABORT:
      if (m_ble_scanner_callback.onScanAbort != NULL) {
        m_ble_scanner_callback.onScanAbort();
      }
    break;
    case NEST_SCANNER_TIMEOUT:
      if (m_ble_scanner_callback.onScanCompleted != NULL) {
        m_ble_scanner_callback.onScanCompleted();
      }
    break;
    case NEST_SCANNER_RESULT:
      memcpy(record.device.address, p_event->record.device.address, 6);
      record.device.type = p_event->record.device.type;
      record.rssi = p_event->record.rssi;
      record.scan_response = p_event->record.scan_response;
      record.type = p_event->record.type;
      record.len = p_event->record.len;
      memcpy(record.data, p_event->record.data, ADVERTISE_MAX_DATA_SIZE);    // Advertising or scan response data.
      if (m_ble_scanner_callback.onScanResult != NULL) {
        m_ble_scanner_callback.onScanResult(&record);
      }
    break;
  }
}
/*---------------------------------------------------------------------------*/
static void
ble_adv_api_irq_hooker(void *ptr)
{
  app_irq_ble_adv_event_t *p_event = (app_irq_ble_adv_event_t *) ptr;
  if (p_event->type == NEST_BROADCASTER_SENDOUT) {
    if (m_ble_adv_callback.onSendout != NULL && m_ble_adv_started) {
      // PRINTF("[ble api] adv sendout\n");
      m_ble_adv_callback.onSendout();
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
ble_api_conn_irq_hooker(void *ptr)
{
  app_irq_ble_conn_event_t *p_event = (app_irq_ble_conn_event_t *) ptr;
  if (m_ble_gatt_server_callback.onConnectionStateChanged != NULL) {
    PRINTF("[ble api] conn irq %d %d\n", p_event->conn_handle, p_event->state);
    m_ble_gatt_server_callback.onConnectionStateChanged(p_event->conn_handle, p_event->state);
  }
}
/*---------------------------------------------------------------------------*/
static void
ble_api_characteristic_irq_hooker(void *ptr)
{

  app_irq_ble_characteristic_event_t *p_event = (app_irq_ble_characteristic_event_t *) ptr;

  PRINTF("[ble api] characteristic event irq %d %d %d %d\n", p_event->type, p_event->conn_handle, p_event->handle, p_event->length);
  PRINTF("[ble api] write data ");
  for (int i = 0; i < p_event->length; i++) {
    PRINTF("0x%02x ", p_event->value[i]);
  }
  PRINTF("\n");

  if (p_event->type == BLE_CHARACTERISTIC_WRITE_REQUEST) {
    if (m_ble_gatt_server_callback.onCharacteristicWriteRequest != NULL) {
        m_ble_gatt_server_callback.onCharacteristicWriteRequest(p_event->conn_handle,
                                      p_event->handle, p_event->value, p_event->length);
    }
  }

}
/*---------------------------------------------------------------------------*/
static int
ble_gap_get_mac_addr(uint8_t *address)
{
  return NEST.gap_local_addr(address);
}
/*---------------------------------------------------------------------------*/
static int
ble_gap_set_adv_tx_power(int8_t power)
{
  m_tx_power = power;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
ble_gap_set_device_name(const char *name, int len)
{
  return NEST.gap_set_device_name(name, len);
}
/*---------------------------------------------------------------------------*/
static int
ble_scan_start_api(ScanIntervalLevel level, const BleScanCallback *gattServerCallback)
{
  if (!m_ble_pheripheral_connected)
    if (m_ble_adv_started )
      return EINVALSTATE;

  //PRINTF("[ble_sysc] startScan\n");
  if (gattServerCallback != NULL) {
   if (gattServerCallback->onScanCompleted != NULL)
     m_ble_scanner_callback.onScanCompleted = gattServerCallback->onScanCompleted;
   if (gattServerCallback->onScanAbort != NULL)
     m_ble_scanner_callback.onScanAbort = gattServerCallback->onScanAbort;
   if (gattServerCallback->onScanResult != NULL)
     m_ble_scanner_callback.onScanResult = gattServerCallback->onScanResult;
  } else {
    return ENULLP;
  }

  if (m_ble_scanner_callback.onScanResult == NULL)
    return EINVAL;

  switch (level) {
    case SCAN_INTERVAL_LEVEL_0:
    case SCAN_INTERVAL_LEVEL_1:
    case SCAN_INTERVAL_LEVEL_2:
    case SCAN_INTERVAL_LEVEL_3:
    case SCAN_INTERVAL_LEVEL_4:
    case SCAN_INTERVAL_LEVEL_5:
    case SCAN_INTERVAL_LEVEL_6:
    case SCAN_INTERVAL_LEVEL_7:
    case SCAN_INTERVAL_LEVEL_8:
      m_ble_scan_interval_level = level;
    break;
    default:
      return EINVAL;
  }

  m_ble_scan_started = true;

  // post to the kernel schedule to process nest controller
  process_post(&nest_ble_scan_api_process, PROCESS_EVENT_CONTINUE, NULL);
  xTaskNotifyGive( g_contiki_thread );
  taskYIELD();
  // PRINTF("[ble api] task notify scan complete\n");
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
ble_scan_stop_api(void)
{
  m_ble_scan_started = false;
  m_ble_scan_interval_level = SCAN_INTERVAL_LEVEL_1;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
ble_adv_stop_api(void)
{
  m_ble_adv_started = false;
  m_ble_adv_interval_level = ADV_INTERVAL_LEVEL_1;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
ble_adv_start_api(AdvIntervalLevel level, const BleAdvertiseCallback *advCallback, const BleGattServerCallback *gattServerCallback)
{
  if (m_ble_scan_started || m_ble_pheripheral_connected)
    return EINVALSTATE;

  switch (level) {
    case ADV_INTERVAL_LEVEL_0:
    case ADV_INTERVAL_LEVEL_1:
    case ADV_INTERVAL_LEVEL_2:
    case ADV_INTERVAL_LEVEL_3:
    case ADV_INTERVAL_LEVEL_4:
    case ADV_INTERVAL_LEVEL_5:
    case ADV_INTERVAL_LEVEL_6:
    case ADV_INTERVAL_LEVEL_7:
    case ADV_INTERVAL_LEVEL_8:
      m_ble_adv_interval_level = level;
    break;
    default:
      return EINVAL;
  }

  if (gattServerCallback != NULL) {
   m_ble_adv_type = ADV_TYPE_ADV_IND;
   if (gattServerCallback->onConnectionStateChanged != NULL)
     m_ble_gatt_server_callback.onConnectionStateChanged = gattServerCallback->onConnectionStateChanged;
   if (gattServerCallback->onCharacteristicReadRequest != NULL)
     m_ble_gatt_server_callback.onCharacteristicReadRequest = gattServerCallback->onCharacteristicReadRequest;
   if (gattServerCallback->onCharacteristicWriteRequest != NULL)
     m_ble_gatt_server_callback.onCharacteristicWriteRequest = gattServerCallback->onCharacteristicWriteRequest;
 }
 else
   m_ble_adv_type = ADV_TYPE_ADV_NONCONN_IND;

  if (advCallback != NULL) {
    if (advCallback->onSendout != NULL)
      m_ble_adv_callback.onSendout = advCallback->onSendout;
  }

  // start advertising
  m_ble_adv_started = true;

  PRINTF("[ble api] process post ble adv api\n");
  process_post(&nest_ble_adv_api_process, PROCESS_EVENT_CONTINUE, NULL);
  xTaskNotifyGive( g_contiki_thread );
  taskYIELD();
  PRINTF("[ble api] task notify complete\n");
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
ble_adv_setdata_api(AdvData *advdata, AdvData *scanrsp_data)
{
  int errcode = ENONE;

  if (advdata == NULL)
    return ENULLP;

  if (scanrsp_data == NULL) {
    errcode = NEST.gap_set_advdata(advdata->data, advdata->size, NULL, 0);
  }
  else {
    errcode = NEST.gap_set_advdata(advdata->data, advdata->size, scanrsp_data->data, scanrsp_data->size);
  }
  if (errcode != ENONE) {
    PRINTF("[ble api] set advdata error %d\n", errcode);
  }
  return errcode;
}
/*---------------------------------------------------------------------------*/
BleManager*
CKBleManager(void)
{
  static BleManager instance;
  instance.getMacAddress = ble_gap_get_mac_addr;
  instance.setTxPower = ble_gap_set_adv_tx_power;
  instance.setDeviceName = ble_gap_set_device_name;
  instance.setAdvertisementData = ble_adv_setdata_api;
  instance.startAdvertising = ble_adv_start_api;
  instance.stopAdvertising = ble_adv_stop_api;
  instance.startScan = ble_scan_start_api;
  instance.stopScan = ble_scan_stop_api;
  instance.addService = ble_gatt_add_service_api;
  instance.notifyCharacteristic = ble_gatts_notify_characteristic;
  return &instance;
}
static struct symbols symbolCKBleManager = {
	.name = "CKBleManager",
	.value = (void *)&CKBleManager
};
static struct symbols symbolCKAdvDataBuilder = {
	.name = "CKAdvDataBuilder",
	.value = (void *)&CKAdvDataBuilder
};
/*---------------------------------------------------------------------------*/
static void
app_terminating(void)
{
  if (m_ble_scan_started)
    m_ble_scan_started = false;

  if (m_ble_adv_started)
    m_ble_adv_started = false;

  if (m_ble_pheripheral_connected)
    m_ble_pheripheral_connected = false;

  // initializes gatt server gattServerCallback
  m_ble_gatt_server_callback.onConnectionStateChanged = NULL;
  m_ble_gatt_server_callback.onCharacteristicReadRequest = NULL;
  m_ble_gatt_server_callback.onCharacteristicWriteRequest = NULL;

  m_ble_scan_interval_level = SCAN_INTERVAL_LEVEL_1;
  m_ble_adv_interval_level = ADV_INTERVAL_LEVEL_1;

  // initializes ble scanner gattServerCallback
  m_ble_scanner_callback.onScanCompleted = NULL;
  m_ble_scanner_callback.onScanAbort = NULL;
  m_ble_scanner_callback.onScanResult = NULL;

  m_ble_adv_callback.onSendout = NULL;

  NEST.gap_set_device_name(PLATFORM_DEVICE_NAME, strlen(PLATFORM_DEVICE_NAME));
}
/*---------------------------------------------------------------------------*/
static struct app_lifecycle_event lifecycle_event = {
	.name = "ble_api",
	.terminating = app_terminating
};
/*---------------------------------------------------------------------------*/
void
ble_api_init(void)
{
  // initializes gatt server gattServerCallback
  m_ble_gatt_server_callback.onConnectionStateChanged = NULL;
  m_ble_gatt_server_callback.onCharacteristicReadRequest = NULL;
  m_ble_gatt_server_callback.onCharacteristicWriteRequest = NULL;

  m_ble_scanner_callback.onScanCompleted = NULL;
  m_ble_scanner_callback.onScanAbort = NULL;
  m_ble_scanner_callback.onScanResult = NULL;

  m_ble_adv_callback.onSendout = NULL;

  app_lifecycle_register(&lifecycle_event);
  symtab_add(&symbolCKBleManager);
  symtab_add(&symbolCKAdvDataBuilder);
}
/*---------------------------------------------------------------------------*/
bool
ble_scan_api_attached(void)
{
  if (m_ble_scan_started) {
    return true;
  }
  return false;
}
/*---------------------------------------------------------------------------*/
uint16_t
ble_scan_api_interval_level(void)
{
  return m_ble_scan_interval_level;
}
/*---------------------------------------------------------------------------*/
void
ble_scan_completed(uint8_t type)
{
  switch (type) {
    case NEST_SCANNER_ABORT:
    {
      if (m_ble_scanner_callback.onScanAbort != NULL) {
        app_irq_event_t irq_event;
        irq_event.event_type = APP_BLE_SCAN_EVENT;
        irq_event.params.ble_scan_event.type = NEST_SCANNER_ABORT;
        irq_event.event_hook = ble_scan_api_irq_hooker;
        xQueueSend( g_app_irq_queue_handle,  &irq_event, ( TickType_t ) 0 );
      }
    }
    break;
    case NEST_SCANNER_TIMEOUT:
    {
      if (m_ble_scanner_callback.onScanCompleted != NULL) {
        app_irq_event_t irq_event;
        irq_event.event_type = APP_BLE_SCAN_EVENT;
        irq_event.params.ble_scan_event.type = NEST_SCANNER_TIMEOUT;
        irq_event.event_hook = ble_scan_api_irq_hooker;
        xQueueSend( g_app_irq_queue_handle,  &irq_event, ( TickType_t ) 0 );
      }
    }
    break;
    default:
    break;
  }
  m_ble_scan_started = false;
}
/*---------------------------------------------------------------------------*/
bool
ble_adv_api_attached(void)
{
  if (m_ble_adv_started) {
    return true;
  }
  return false;
}
/*---------------------------------------------------------------------------*/
void
ble_adv_api_sendout(void)
{
  /* Now the buffer is empty we can switch context if necessary. */
  if (m_ble_adv_callback.onSendout != NULL && m_ble_adv_started) {
    app_irq_event_t irq_event;
    irq_event.event_type = APP_BLE_ADV_EVENT;
    irq_event.params.ble_adv_event.type = NEST_BROADCASTER_SENDOUT;
    irq_event.event_hook = ble_adv_api_irq_hooker;
    xQueueSend( g_app_irq_queue_handle,  &irq_event, ( TickType_t ) 0 );
  }
}
/*---------------------------------------------------------------------------*/
void
ble_adv_api_params_retrieve(nest_broadcast_params_t *params)
{
  params->type = m_ble_adv_type;
  params->interval = m_ble_adv_interval_level;
}
/*---------------------------------------------------------------------------*/
void
ble_scan_api_event_report(nest_scanner_peer_t *report)
{
  static app_irq_ble_scan_event_t event;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  /* Now the buffer is empty we can switch context if necessary. */
  if (m_ble_scanner_callback.onScanResult != NULL && m_ble_scan_started) {
    // copy data to the evnet record
    event.type = NEST_SCANNER_RESULT;
    memcpy(event.record.device.address, report->peer_address, 6);
    event.record.device.type = report->peer_addr_type;
    event.record.rssi = report->rssi;
    event.record.scan_response = report->scan_response;
    event.record.type = report->type;
    event.record.len = report->len;
    memcpy(event.record.data, report->data, ADVERTISE_MAX_DATA_SIZE);    // Advertising or scan response data.

    app_irq_event_t irq_event;
    irq_event.event_type = APP_BLE_SCAN_EVENT;
    memcpy(&(irq_event.params.ble_scan_event), &event, sizeof(app_irq_ble_scan_event_t));
    irq_event.event_hook = ble_scan_api_irq_hooker;

		//xQueueSend( g_app_irq_queue_handle,  &irq_event, ( TickType_t ) 0 );
    xQueueSendFromISR( g_app_irq_queue_handle, &irq_event, &xHigherPriorityTaskWoken );
    if( xHigherPriorityTaskWoken )  {
        portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
  }
}
/*---------------------------------------------------------------------------*/
void ble_gatts_write_event_handler(uint16_t conn_handle, uint16_t handle,
                                                uint8_t *value, uint16_t length)
{
  static app_irq_ble_characteristic_event_t event;
  if (m_ble_gatt_server_callback.onCharacteristicWriteRequest != NULL) {
    app_irq_event_t irq_event;
    irq_event.event_type = APP_BLE_CHARACTERISTIC_EVENT;
    // copy params
    event.type = BLE_CHARACTERISTIC_WRITE_REQUEST;
    event.conn_handle = conn_handle;
    event.handle = handle;
    memcpy(event.value, value, 20);
    event.length = length;
    memcpy(&(irq_event.params.ble_characteristic_event), &event, sizeof(app_irq_ble_characteristic_event_t));
    irq_event.event_hook = ble_api_characteristic_irq_hooker;
    xQueueSend( g_app_irq_queue_handle,  &irq_event, ( TickType_t ) 0 );
  }
}
/*---------------------------------------------------------------------------*/
void
ble_api_connection_event_handler(uint16_t conn_handle, uint16_t state)
{
  static app_irq_ble_conn_event_t event;

  PRINTF("[ble api] connection event %d %d\n", conn_handle, state);

  if (state == BLE_GATT_STATE_CONNECTED) {
    m_ble_pheripheral_connected = true;
  } else if (state == BLE_GATT_STATE_DISCONNECTED) {
    m_ble_pheripheral_connected = false;
  }

  if (m_ble_gatt_server_callback.onConnectionStateChanged != NULL) {
    app_irq_event_t irq_event;
    irq_event.event_type = APP_BLE_CONN_EVENT;
    event.conn_handle = conn_handle;
    event.state = state;
    memcpy(&(irq_event.params.ble_conn_event), &event, sizeof(app_irq_ble_conn_event_t));
    irq_event.event_hook = ble_api_conn_irq_hooker;
    xQueueSend( g_app_irq_queue_handle,  &irq_event, ( TickType_t ) 0 );
  }
}
/*---------------------------------------------------------------------------*/
int8_t
ble_api_get_tx_power(void)
{
  return m_tx_power;
}
/*---------------------------------------------------------------------------*/
