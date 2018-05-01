/**
 *  Copyright (c) 2018 AlfaLoop Technology Co., Ltd. All Rights Reserved.
 *
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential.
 *
 *  Attribution - You must give appropriate credit, provide a link to the license, and
 *  indicate if changes were made. You may do so in any reasonable manner, but not in any
 *  way that suggests the licensor endorses you or your use.
 *
 *  NonCommercial - You may not use the material for commercial purposes under unauthorized.
 *
 *  NoDerivatives - If you remix, transform, or build upon the material, you may not
 *  distribute the modified material.
 */
#include "contiki.h"
#include "nest-central.h"

#if defined(UED_WDUI_STACK)
#include "wdui/wdui.h"
#endif
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

typedef struct {
  uint8_t     *p_data;
  uint16_t     data_len;
} data_t;

/*---------------------------------------------------------------------------*/
static bool 				 	 					 m_inbound_characteristic_found = false;
static bool 				 	 					 m_outbound_characteristic_found = false;
static nest_blecharacteristic_t  m_inbound_characteristic;
static nest_blecharacteristic_t  m_outbound_characteristic;
static int                       m_central_status = NEST_CENTRAL_STATUS_NONE;
static uint16_t                  m_central_conn_id = 0;
static uint8_t                   m_adv_nfd_uuid[16] = BLE_UUID_NEST_VENDOR_128UUID;
static uint8_t                   m_adv_dfd_uuid[16] = BLE_UUID_NEST_VENDOR_128UUID;

/*---------------------------------------------------------------------------*/
static uint8_t
peer_adv_report_parse(uint8_t type, data_t *p_advdata, data_t *p_typedata)
{
  uint32_t  index = 0;
  uint8_t * p_data;

  p_data = p_advdata->p_data;

  while (index < p_advdata->data_len)
  {
    uint8_t field_length = p_data[index];
    uint8_t field_type   = p_data[index + 1];

    if (field_type == type)
    {
			p_typedata->p_data   = &p_data[index + 2];
			p_typedata->data_len = field_length - 1;
      return 1;
    }
    index += field_length + 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static void
gattc_write_completed_event(void)
{
  // TODO: crypto challenge
  if (m_central_status == NEST_CENTRAL_STATUS_ENABLE_NOTIFICATION) {
    m_central_status = NEST_CENTRAL_STATUS_PROCESS_COMMAND;
    PRINTF("[nest central] start process command\n");
  } else if (m_central_status == NEST_CENTRAL_STATUS_PROCESS_COMMAND) {
    if (process_is_running(&nest_central_send_process)) {
      process_post(&nest_central_send_process, PROCESS_EVENT_POLL, NULL);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
gatt_service_discovery_completed(uint16_t conn_id, uint8_t characteristic_count)
{
  uint32_t err_code;
  nest_bleuuid_t service_uuid;
  uint16_t i;
  nest_blecharacteristic_t blechar;
  uint8_t buf[BLE_CCCD_HANDLE_VALUE_LEN];

  service_uuid.uuid = BLE_UUID_NEST_SERVICE;
  service_uuid.type = NEST_BLE_UUID_TYPE_VENDOR;

  if (m_central_conn_id == conn_id) {

    if (m_central_status == NEST_CENTRAL_STATUS_SERVICE_DISCOVERY) {
      if (characteristic_count == 2) {
        for (i = 0; i < characteristic_count ; i++) {
          // PRINTF("[nest central] gattc get service %d, uuid 0x%04x, index %d\n", conn_id, service_uuid.uuid, i );
          err_code = NEST.gattc_get_service(conn_id, &service_uuid, i, &blechar);
          if (err_code != ENONE) {
            // PRINTF("[nest central] characteristic not found, disconnect\n");
            NEST.gap_disconnect(conn_id);
            m_central_status = NEST_CENTRAL_STATUS_NONE;
          } else {
            // PRINTF("[nest central] found characteristic UUID 0x%04x\n", blechar.uuid);
            if (blechar.uuid == BLE_UUID_NEST_INBOUND_CHARACTERISTIC) {
              memcpy(&m_inbound_characteristic, &blechar, sizeof(nest_blecharacteristic_t));
              m_inbound_characteristic_found = true;
              // PRINTF("[nest central] inbound characteristic found handle number %d\n", blechar.value_handle);
            } else if (blechar.uuid == BLE_UUID_NEST_OUTBOUND_CHARACTERISTIC) {
              memcpy(&m_outbound_characteristic, &blechar, sizeof(nest_blecharacteristic_t));
              m_outbound_characteristic_found = true;
              // PRINTF("[nest central] outbound characteristic found handle number %d\n", blechar.value_handle);
            }
          }
        }

        // enabling notificaiotn of inbound characteristic
        if (m_outbound_characteristic_found && m_inbound_characteristic_found) {

          // enable notificaiotn
          buf[0] = NEST_BLE_HVX_TYPE_NOTIFICATION;
          buf[1] = 0;
          m_central_status = NEST_CENTRAL_STATUS_ENABLE_NOTIFICATION;

          PRINTF("[nest central] enable notificaiotn of inbound characteristic\n");
          err_code = NEST.gattc_write(conn_id, m_inbound_characteristic.cccd_handle,
                                               0, BLE_CCCD_HANDLE_VALUE_LEN, buf, gattc_write_completed_event);
          if (err_code != ENONE) {
            //PRINTF("[nest channel] gattc enabling notification failed %d\n", err_code);
            NEST.gap_disconnect(conn_id);
            nest_central_init();
          }
        }
      } else {
        NEST.gap_disconnect(conn_id);
        nest_central_init();
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
gatt_service_discovery_abort(uint16_t conn_id)
{
  PRINTF("[nest central] service discovery abort %d\n", conn_id);
  if (m_central_conn_id == conn_id) {
    NEST.gap_disconnect(m_central_conn_id);
    nest_central_init();
  }
}
/*---------------------------------------------------------------------------*/
PROCESS(nest_central_send_process, "Nest central send");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_central_send_process, ev, data)
{
  static struct etimer et;
  static uint32_t err_code;
  static nest_command_data_t *output;
  static uint8_t txbuf[BLE_NEST_FIX_DATA_LEN];

  PROCESS_BEGIN();

  output = data;

  nest_packer_pack(&txbuf[0], output->len, output->data, output->opcode | 0x80);

  err_code = NEST.gattc_write(m_central_conn_id, m_outbound_characteristic.value_handle,
            0, BLE_NEST_FIX_DATA_LEN, txbuf, gattc_write_completed_event);
  if (err_code != ENONE) {
    PRINTF("[nest channel] gattc write failed %d\n", err_code);
  } else {
    etimer_set(&et, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL( (ev == PROCESS_EVENT_POLL) ||
                              (etimer_expired(&et)) );
    etimer_stop(&et);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
const nest_service_discovery_callback_t service_discovery_callback = {
  .completed = gatt_service_discovery_completed,
  .abort = gatt_service_discovery_abort
};
/*---------------------------------------------------------------------------*/
static void
initiate_connect_timeout_event_handler(void)
{
  PRINTF("[nest central] initiate connection timeout\n");
	nest_central_init();
}
/*---------------------------------------------------------------------------*/
void
nest_central_init(void)
{
  uint8_t mac[6];

	m_central_status = NEST_CENTRAL_STATUS_NONE;
	m_inbound_characteristic_found = false;
	m_outbound_characteristic_found = false;
  memset(&m_inbound_characteristic, 0x00, sizeof(nest_blecharacteristic_t));
  memset(&m_outbound_characteristic_found, 0x00, sizeof(nest_blecharacteristic_t));
  m_central_conn_id = 0;

  // setup the dfd packets
  NEST.gap_local_addr(mac);
  m_adv_dfd_uuid[0] = mac[0];
  m_adv_dfd_uuid[1] = mac[1];
  m_adv_dfd_uuid[2] = mac[2];
  m_adv_dfd_uuid[3] = mac[3];
  m_adv_dfd_uuid[4] = mac[4];
  m_adv_dfd_uuid[5] = mac[5];
}
/*---------------------------------------------------------------------------*/
void
nest_central_initiate_connection(uint8_t *peer_addr, uint8_t peer_type)
{
	uint32_t err_code;
	nest_device_t peer;

	if (m_central_status == NEST_CENTRAL_STATUS_NONE) {
    peer.type = peer_type;
    memcpy(peer.address, peer_addr, BLE_DEVICE_ADDR_LEN);
    memset(&m_inbound_characteristic, 0x00, sizeof(nest_blecharacteristic_t));
    memset(&m_outbound_characteristic_found, 0x00, sizeof(nest_blecharacteristic_t));
		err_code = NEST.gap_connect(&peer, initiate_connect_timeout_event_handler);
		if (err_code != ENONE) {
			PRINTF("[nest central] initiate connection failed, reason %d\n", err_code);
		} else {
			m_central_status = NEST_CENTRAL_STATUS_INITIATING;
		}
	}
}
/*---------------------------------------------------------------------------*/
void
nest_central_connected_event(uint16_t conn_id, uint8_t *address, uint8_t type)
{
  PRINTF("[nest central] connected %d\n", conn_id);
  uint32_t err_code;
  nest_bleuuid_t service_uuid;

  service_uuid.uuid = BLE_UUID_NEST_SERVICE;
  service_uuid.type = NEST_BLE_UUID_TYPE_VENDOR;

  m_central_conn_id = conn_id;

  // start to discover services
  err_code = NEST.gatt_start_service_discovery(m_central_conn_id, &service_uuid, &service_discovery_callback);
  if (err_code != ENONE) {
    //PRINTF("[nest central] serivce discovery failed %d\n", err_code);
    NEST.gap_disconnect(m_central_conn_id);
    nest_central_init();
  } else {
    m_central_status = NEST_CENTRAL_STATUS_SERVICE_DISCOVERY;
  }
}
/*---------------------------------------------------------------------------*/
void
nest_central_disconnect_event(uint16_t conn_id, uint8_t reason)
{
  PRINTF("[nest central] disconnect %d reason %d\n", conn_id, reason);
  nest_central_init();
}
/*---------------------------------------------------------------------------*/
void
nest_central_gattc_handle_value_event(uint16_t conn_id, uint16_t handle, const uint8_t *data, uint16_t length)
{
  nest_command_data_t input;
	nest_proto_header_t header;

  // #if DEBUG_MODULE > 1
  PRINTF("[nest central] gattc handle value length: %d data ", length);
	for (int i = 0; i < length; i++) {
		PRINTF("%02x ", data[i]);
	}
	PRINTF("\n");
  PRINTF("[nest central] conn_id: %d handle %d central status %d\n", conn_id, handle, m_central_status);
  PRINTF("[nest central] outbound: %d inbound %d\n", m_outbound_characteristic_found, m_inbound_characteristic_found);
  // #endif

	if (length != BLE_NEST_FIX_DATA_LEN) {
		return;
	}

  if (m_central_status == NEST_CENTRAL_STATUS_PROCESS_COMMAND) {
    if ( (m_outbound_characteristic_found && m_inbound_characteristic_found) &&
         (conn_id == m_central_conn_id) )
    {
      if (m_inbound_characteristic.value_handle == handle) {
        if (!nest_packer_unpack(data, input.data, &header))  {
        	PRINTF("[nest central] unpack error\n");
          // terminate connection.
          NEST.gap_disconnect(conn_id);
          nest_central_init();
          return;
        }

        input.opcode = header.opcode;
        input.len = header.len;
        input.response_process = &nest_central_send_process;
        nest_command_inbound(&input);
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
uint8_t
nest_central_adv_filter(uint8_t *src, uint8_t length)
{
  uint32_t err_code;
  uint8_t  xor_value = 0;
  data_t   adv_data;
  data_t   service_uuid;
  data_t   service_data;
  data_t   davicename;
  bool  service_uuid_exist;
  bool  service_data_exist;
  bool  davicename_exist;

  // Initialize advertisement report for parsing.
  adv_data.p_data     = src;
  adv_data.data_len   = length;

  service_uuid_exist = peer_adv_report_parse(ADV_TYPE_128BIT_SERVICE_UUID_COMPLETE,
                              &adv_data,
                              &service_uuid);
  service_data_exist = peer_adv_report_parse(ADV_TYPE_SERVICE_DATA,
                              &adv_data,
                              &service_data);
  davicename_exist = peer_adv_report_parse(ADV_TYPE_COMPLETE_LOCAL_NAME,
                              &adv_data,
                              &davicename);

  if (service_uuid_exist ) {
    if (service_uuid.data_len == 16) {
// #if DEBUG_MODULE > 1
      PRINTF("[nest central] nest service uuid %d: ", service_uuid.data_len);
      for (uint8_t i = 0; i < service_uuid.data_len; i++) {
        PRINTF("%02x ", service_uuid.p_data[i]);
      }
      PRINTF("\n");
// #endif
      if (!(memcmp(m_adv_nfd_uuid, service_uuid.p_data, 16))) {
        return NEST_CENTRAL_ADV_TYPE_NFD;
      }

      if (!(memcmp(m_adv_dfd_uuid, service_uuid.p_data, 16))) {
        return NEST_CENTRAL_ADV_TYPE_DFD;
      }
    }
  }
  return NEST_CENTRAL_ADV_TYPE_UNKNOWN;
}
/*---------------------------------------------------------------------------*/
uint8_t
nest_central_status(void)
{
  return m_central_status;
}
/*---------------------------------------------------------------------------*/
