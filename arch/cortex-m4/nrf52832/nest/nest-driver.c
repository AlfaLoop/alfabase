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
#include "app_util.h"
#include "app_error.h"
#include "nrf.h"
#include "nrf_soc.h"
#include "nrf_sdm.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_types.h"
#include "ble_conn_params.h"
#include "ble_conn_state.h"
#include "nordic_common.h"
#include "softdevice_handler.h"
#include "softdevice_arch.h"
#include "ble_radio_notification.h"
#include "errno.h"

#if defined(USE_FREERTOS)
#include "FreeRTOS.h"
#include "task.h"
#endif
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
#define SCAN_QUEUE_SIZE                      4
#define APP_TIMER_PRESCALER                  0                                         /**< Value of the RTC1 PRESCALER register. */
#define MSEC_TO_UNITS(TIME, RESOLUTION) (((TIME) * 1000) / (RESOLUTION))

#define ROUNDED_DIV(A, B) (((A) + ((B) / 2)) / (B))
#define APP_TIMER_CLOCK_FREQ         32768
#define APP_TIMER_TICKS(MS, PRESCALER)\
            ((uint32_t)ROUNDED_DIV((MS) * (uint64_t)APP_TIMER_CLOCK_FREQ, ((PRESCALER) + 1) * 1000))

#define NRF_BLE_MAX_MTU_SIZE            GATT_MTU_SIZE_DEFAULT             /**< MTU size used in the softdevice enabling and to reply to a BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST event. */


#define FIRST_CONN_PARAMS_UPDATE_DELAY   5* CLOCK_SECOND 		/**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (20 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   20* CLOCK_SECOND 			/**< Time between each call to sd_ble_gap_conn_param_update after the first call (5 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3              /**< Number of attempts before giving up the connection parameter negotiation. */

#define SRV_DISC_START_HANDLE  0x0001                    /**< The start handle value used during service discovery. */
#define BLE_GATT_DB_MAX_CHARS 5       /**< The maximum number of characteristics present in a service record. */
#define APP_FEATURE_NOT_SUPPORTED        BLE_GATT_STATUS_ATTERR_APP_BEGIN + 2        /**< Reply when unsupported features are requested. */

// Structure for holding the characteristic and the handle of its CCCD present on a server.
typedef struct {
  ble_gattc_char_t characteristic;  /**< Structure containing information about the characteristic. */
  uint16_t         cccd_handle;     /**< CCCD Handle value for this characteristic. This will be set to BLE_GATT_HANDLE_INVALID if a CCCD is not present at the server. */
} ble_gatt_db_char_t;

// Structure for holding information about the service and the characteristics present on aserver.
typedef struct {
  ble_uuid_t               srv_uuid;                                  /**< UUID of the service. */
  uint8_t                  char_count;                                /**< Number of characteristics present in the service. */
  ble_gattc_handle_range_t handle_range;                              /**< Service Handle Range. */
  ble_gatt_db_char_t       charateristics[BLE_GATT_DB_MAX_CHARS];     /**< Array of information related to the characteristics present in the service. This list can extend further than one. */
} ble_gatt_db_srv_t;

typedef struct {
  ble_gatt_db_srv_t   service;                  /**< Information related to the current service being discovered. This is intended for internal use during service discovery.*/
  uint8_t             curr_char_ind;            /**< Index of the current characteristic being discovered. This is intended for internal use during service discovery.*/
  bool                discovery_in_progress;    /**< Variable to indicate if there is a service discovery in progress. */
  uint16_t            conn_handle;              /**< Connection handle on which the discovery is started*/
} ble_db_discovery_t;

typedef struct {
  uint16_t conn_id;
  uint8_t  characteristic_count;
} ble_service_discovery_event_t;

typedef struct {
  uint16_t conn_id;
  uint8_t role;
  uint8_t address[6];
  uint8_t type;
} nrf_gap_connected_event_t;

typedef struct {
  uint16_t conn_id;
  uint8_t reason;
} nrf_gap_disconnect_event_t;

typedef struct {
  uint16_t  conn_id;
  uint16_t  handle;
  uint8_t   data[BLE_NEST_FIX_DATA_LEN];
  uint16_t  length;
} nrf_gatt_write_value_event_t;

typedef enum { FREE=0x0, INPUT, INPUT_ALLOC } scqueue_types_t;

/* storage for scanned packetd */
static nest_scanner_peer_t scanbuf[SCAN_QUEUE_SIZE] ;
typedef struct {
	uint8_t head, tail, types[SCAN_QUEUE_SIZE];
} scq_t;
static scq_t scq;

/*---------------------------------------------------------------------------*/
static bool                 m_radio_active = false;
// list of DB structures used by the database discovery module.
static ble_db_discovery_t   m_ble_db_discovery[NEST_CENTRAL_LINK_COUNT + NEST_PERIPHERAL_LINK_COUNT];
static ble_service_discovery_event_t m_ble_service_discovery_params;
static nest_service_discovery_callback_t m_service_discovery_callback;
/*---------------------------------------------------------------------------*/
static void
scq_head_complete(void)
{
	switch(scq.types[scq.head])
	{
		case INPUT_ALLOC:
			scq.types[scq.head] = INPUT;
		break;
		case FREE:
		break;
		default:
		break;
	}
}
/*---------------------------------------------------------------------------*/
static void
scq_enqueue(nest_scanner_peer_t *ind, scqueue_types_t type)
{
	if(scq.types[scq.tail] == FREE)
	{
		scq.types[scq.tail] = type;
		scanbuf[scq.tail].peer_addr_type = ind->peer_addr_type;
		scanbuf[scq.tail].rssi = ind->rssi;
    scanbuf[scq.tail].type = ind->type;
    scanbuf[scq.tail].len = ind->len;
		scanbuf[scq.tail].scan_response = ind->scan_response;
		memcpy(scanbuf[scq.tail].data, ind->data, ADV_MAX_DATA_SIZE);
    memcpy(scanbuf[scq.tail].peer_address, ind->peer_address, 6);
		scq.tail = (scq.tail+1)%SCAN_QUEUE_SIZE;
		// PRINTF("[nest command] scq enqueue tail=%d \n", scq.tail);
	}
}
/*---------------------------------------------------------------------------*/
static void
scq_dequeue()
{
	if(scq.types[scq.head] == INPUT)
	{
		scq.types[scq.head] = FREE;
		scq.head = (scq.head+1)%SCAN_QUEUE_SIZE;
	}
}
/*---------------------------------------------------------------------------*/
/* rx queue handling functions */
static nest_scanner_peer_t*
scq_peek()
{
	// PRINTF("[nest command] scq peak head=%d type=%d\n", scq.head, scq.types[scq.head]);
  if(scq.types[scq.head]== FREE ||
		 scq.types[scq.head]== INPUT_ALLOC )
    return NULL;

  return (nest_scanner_peer_t*) &scanbuf[scq.head];
}
/*---------------------------------------------------------------------------*/
static scqueue_types_t
scq_peektype()
{
	return scq.types[scq.head];
}
/*---------------------------------------------------------------------------*/
PROCESS(nrf_service_discovery_process, "nrf service_discovery");
PROCESS(nrf_sd_event_process, "nrf gap connect");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nrf_service_discovery_process, ev, data)
{
  PROCESS_BEGIN();
  PROCESS_WAIT_EVENT_UNTIL( (ev == nrf_sd_event_services_discovered) ||
                            (ev == PROCESS_EVENT_POLL) );
  if (ev == nrf_sd_event_services_discovered) {
    if ( m_service_discovery_callback.completed != NULL)
      m_service_discovery_callback.completed(m_ble_service_discovery_params.conn_id,
              m_ble_service_discovery_params.characteristic_count);
  } else if (ev == PROCESS_EVENT_POLL) {
    if ( m_service_discovery_callback.abort != NULL)
      m_service_discovery_callback.abort(m_ble_service_discovery_params.conn_id);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nrf_sd_event_process, ev, data)
{
  static nrf_gap_connected_event_t *connected_params;
  static nrf_gap_disconnect_event_t *disconnect_params;
  static nrf_gatt_write_value_event_t  *gattc_write_event_params;
  static nrf_gatt_write_value_event_t  *gatts_write_event_params;
  static nest_scanner_peer_t *scanned_params;

  PROCESS_BEGIN();
  while (1) {
    PROCESS_WAIT_EVENT_UNTIL( (ev == nrf_sd_event_gap_connected)     ||
                              (ev == nrf_sd_event_gap_disconnect)    ||
                              (ev == nrf_sd_event_gattc_hvx)         ||
                              (ev == nrf_sd_event_gatts_write)       ||
                              (ev == nrf_sd_event_tx_completed)      ||
                              (ev == nrf_sd_event_gap_connect_timeout) ||
                              (ev == nrf_sd_event_adv_report)  );
    if (ev == nrf_sd_event_gap_connected) {
      connected_params = (nrf_gap_connected_event_t *)data;
      nest_connected_event(connected_params->conn_id, connected_params->role,
          connected_params->address, connected_params->type);
    } else if (ev == nrf_sd_event_gap_disconnect) {
      disconnect_params = (nrf_gap_disconnect_event_t *)data;
      nest_disconnect_event(disconnect_params->conn_id, disconnect_params->reason);
    } else if (ev == nrf_sd_event_gattc_hvx) {
      gattc_write_event_params = (nrf_gatt_write_value_event_t *)data;
      nest_gattc_handle_value_event(gattc_write_event_params->conn_id,
                                    gattc_write_event_params->handle,
                                    gattc_write_event_params->data,
                                    gattc_write_event_params->length);
    } else if (ev == nrf_sd_event_gatts_write) {
      gatts_write_event_params = (nrf_gatt_write_value_event_t *)data;
      nest_gatts_write_event(gatts_write_event_params->conn_id,
                             gatts_write_event_params->handle,
                             gatts_write_event_params->data,
                             gatts_write_event_params->length);
    } else if (ev == nrf_sd_event_gap_connect_timeout) {
      nrf_gap_central_connect_timeout();
    } else if (ev == nrf_sd_event_tx_completed) {
      nrf_gattc_write_tx_complete();
    } else if (ev == nrf_sd_event_adv_report) {
      do {
        scq_head_complete();
        if ( scq_peektype() == INPUT ) {
          scanned_params = scq_peek();
          if (scanned_params != NULL) {
            nest_scanner_result_event(scanned_params);
          }
        }
        scq_dequeue();
      } while( scq_peektype() != FREE );
    }
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void
ble_on_radio_active_evt(bool radio_active)
{
  m_radio_active = radio_active;
}
/*---------------------------------------------------------------------------*/
static bool
is_desc_discovery_reqd(ble_db_discovery_t       * p_db_discovery,
                       ble_gatt_db_char_t       * p_curr_char,
                       ble_gatt_db_char_t       * p_next_char,
                       ble_gattc_handle_range_t * p_handle_range)
{
  if (p_next_char == NULL)
  {
      // Current characteristic is the last characteristic in the service. Check if the value
      // handle of the current characteristic is equal to the service end handle.
      if (
          p_curr_char->characteristic.handle_value ==
          p_db_discovery->service.handle_range.end_handle
         )
      {
          // No descriptors can be present for the current characteristic. p_curr_char is the last
          // characteristic with no descriptors.
          return false;
      }

      p_handle_range->start_handle = p_curr_char->characteristic.handle_value + 1;

      // Since the current characteristic is the last characteristic in the service, the end
      // handle should be the end handle of the service.
      p_handle_range->end_handle =
          p_db_discovery->service.handle_range.end_handle;

      return true;
  }

  // p_next_char != NULL. Check for existence of descriptors between the current and the next
  // characteristic.
  if ((p_curr_char->characteristic.handle_value + 1) == p_next_char->characteristic.handle_decl)
  {
      // No descriptors can exist between the two characteristic.
      return false;
  }

  p_handle_range->start_handle = p_curr_char->characteristic.handle_value + 1;
  p_handle_range->end_handle   = p_next_char->characteristic.handle_decl - 1;

  return true;
}
/*---------------------------------------------------------------------------*/
static uint32_t
nest_bleservice_init(void)
{
	uint32_t   err_code;
  ble_uuid128_t   core_base_uuid = BLE_UUID_NEST_VENDOR_128UUID;

  ble_uuid_t service_uuid;
  service_uuid.uuid 	= BLE_UUID_NEST_SERVICE;

	// Add a vendor specfit 128-bit UUID
	err_code = sd_ble_uuid_vs_add(&core_base_uuid, &(service_uuid.type));
	if (err_code != NRF_SUCCESS){
    PRINTF("[nest driver] add vendor specfic 128-bit UUID error %d \n", err_code);
    return err_code;
  }
  // PRINTF("[nest driver] add vendor specfic 128-bit tpye:%d\n", service_uuid.type);
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static void
on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
  uint32_t err_code;
  if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_SUCCEEDED) {
    PRINTF("[nest driver] conn params success event\n");
  } else if(p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED) {
    PRINTF("[nest driver] conn params success failed\n");
    err_code = sd_ble_gap_disconnect(p_evt->conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
  	if (err_code != NRF_SUCCESS) {
  		PRINTF("[nest driver] on_conn_params_evt sd_ble_gap_disconnect %d\n", err_code);
  	}
  }
}
/*---------------------------------------------------------------------------*/
// Function for handling errors from the Connection Parameters module.
static void
conn_params_error_handler(uint32_t nrf_error)
{
	if (nrf_error != NRF_SUCCESS) {
		PRINTF("[nest driver] conn_params_error_handler %d\n", nrf_error);
	}
}
/*---------------------------------------------------------------------------*/
static void
conn_params_init(void)
{
  uint32_t               err_code;
  ble_conn_params_init_t cp_init;

  memset(&cp_init, 0, sizeof(cp_init));

  cp_init.p_conn_params                  = NULL;
  cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
  cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
  cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
  cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
  cp_init.disconnect_on_fail             = false;
  cp_init.evt_handler                    = on_conn_params_evt;
  cp_init.error_handler                  = conn_params_error_handler;

  err_code = ble_conn_params_init(&cp_init);
  APP_ERROR_CHECK(err_code);
}
/*---------------------------------------------------------------------------*/
static void
gap_params_init(void)
{
  uint32_t                err_code;
  ble_gap_conn_params_t   gap_conn_params;
  ble_gap_conn_sec_mode_t sec_mode;

  BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);
  err_code = sd_ble_gap_device_name_set(&sec_mode,
                                        (const uint8_t *)PLATFORM_DEVICE_NAME,
                                        strlen(PLATFORM_DEVICE_NAME));
  PRINTF("[nest driver] device name %s\n", PLATFORM_DEVICE_NAME);
	if (err_code != NRF_SUCCESS) {
		PRINTF("[nest driver] sd_ble_gap_device_name_set error: %d\n", err_code);
		while(1);
	}

  memset(&gap_conn_params, 0, sizeof(gap_conn_params));

  /* Default connection parameters */
  gap_conn_params.min_conn_interval = GAP_CONF_MIN_CONN_INTERVAL;
  gap_conn_params.max_conn_interval = GAP_CONF_MAX_CONN_INTERVAL;
  gap_conn_params.slave_latency     = GAP_CONF_SLAVE_LATENCY;
  gap_conn_params.conn_sup_timeout  = GAP_CONF_CONN_SUP_TIMEOUT;

  err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
	if (err_code != NRF_SUCCESS) {
		PRINTF("[nest driver] sd_ble_gap_ppcp_set error: %d\n", err_code);
		while(1);
	}
	return ENONE;
}
/*---------------------------------------------------------------------------*/
static bool
is_char_discovery_reqd(ble_db_discovery_t * const p_db_discovery,
                       ble_gattc_char_t         * p_after_char)
{
  if ( p_after_char->handle_value <
       p_db_discovery->service.handle_range.end_handle)
  {
      // Handle value of the characteristic being discovered is less than the end handle of
      // the service being discovered. There is a possibility of more characteristics being
      // present. Hence a characteristic discovery is required.
      return true;
  }
  return false;
}
/*---------------------------------------------------------------------------*/
static uint32_t
descriptors_discover(ble_db_discovery_t * const p_db_discovery,
                     bool *                     p_raise_discov_complete,
                     uint16_t const             conn_handle)
{
  uint32_t err_code;
  ble_gattc_handle_range_t   handle_range;
  ble_gatt_db_char_t       * p_curr_char_being_discovered;
  ble_gatt_db_srv_t        * p_srv_being_discovered;
  bool                       is_discovery_reqd = false;

  p_srv_being_discovered = &(p_db_discovery->service);

  p_curr_char_being_discovered =
      &(p_srv_being_discovered->charateristics[p_db_discovery->curr_char_ind]);

  if ((p_db_discovery->curr_char_ind + 1) == p_srv_being_discovered->char_count){
    // This is the last characteristic of this service.
    is_discovery_reqd = is_desc_discovery_reqd(p_db_discovery,
                                               p_curr_char_being_discovered,
                                               NULL,
                                               &handle_range);
  }
  else
  {
    uint8_t                   i;
    ble_gatt_db_char_t * p_next_char;
    for (i = p_db_discovery->curr_char_ind;
         i < p_srv_being_discovered->char_count;
         i++)
    {

      if (i == (p_srv_being_discovered->char_count - 1)){
        // The current characteristic is the last characteristic in the service.
        p_next_char = NULL;
      } else {
        p_next_char = &(p_srv_being_discovered->charateristics[i + 1]);
      }

      // Check if it is possible for the current characteristic to have a descriptor.
      if (is_desc_discovery_reqd(p_db_discovery,
                                 p_curr_char_being_discovered,
                                 p_next_char,
                                 &handle_range))
      {
        is_discovery_reqd = true;
        break;
      }
      else
      {
        // No descriptors can exist.
        p_curr_char_being_discovered = p_next_char;
        p_db_discovery->curr_char_ind++;
      }
    }
  }

  if (!is_discovery_reqd)
  {
    // No more descriptor discovery required. Discovery is complete.
    // This informs the caller that a discovery complete event can be triggered.
    *p_raise_discov_complete = true;

    return NRF_SUCCESS;
  }

  *p_raise_discov_complete = false;

  err_code = sd_ble_gattc_descriptors_discover(conn_handle, &handle_range);
  if (err_code == BLE_ERROR_INVALID_CONN_HANDLE) {
    PRINTF("[nest driver] characteristics_discover error invalid conn handle\n");
  } else if (err_code == NRF_ERROR_INVALID_STATE) {
    PRINTF("[nest driver] characteristics_discover error invalid state\n");
  } else if (err_code == NRF_ERROR_INVALID_ADDR) {
    PRINTF("[nest driver] characteristics_discover error invalid address\n");
  } else if (err_code == NRF_ERROR_BUSY) {
    PRINTF("[nest driver] characteristics_discover error busy\n");
  }
  return err_code;
}
/*---------------------------------------------------------------------------*/
static uint32_t
characteristics_discover(ble_db_discovery_t * const p_db_discovery, uint16_t const conn_handle)
{
  uint32_t err_code;
  ble_gattc_handle_range_t handle_range;

  if (p_db_discovery->curr_char_ind != 0)
  {
    // This is not the first characteristic being discovered. Hence the 'start handle' to be
    // used must be computed using the handle_value of the previous characteristic.
    ble_gattc_char_t * p_prev_char;
    uint8_t            prev_char_ind = p_db_discovery->curr_char_ind - 1;

    p_prev_char = &(p_db_discovery->service.charateristics[prev_char_ind].characteristic);
    handle_range.start_handle = p_prev_char->handle_value + 1;
  }
  else
  {
    // This is the first characteristic of this service being discovered.
    handle_range.start_handle = p_db_discovery->service.handle_range.start_handle;
  }

  handle_range.end_handle = p_db_discovery->service.handle_range.end_handle;
  err_code = sd_ble_gattc_characteristics_discover(conn_handle, &handle_range);
  if (err_code == BLE_ERROR_INVALID_CONN_HANDLE) {
    PRINTF("[nest driver] characteristics_discover error invalid conn handle\n");
  } else if (err_code == NRF_ERROR_INVALID_STATE) {
    PRINTF("[nest driver] characteristics_discover error invalid state\n");
  } else if (err_code == NRF_ERROR_INVALID_ADDR) {
    PRINTF("[nest driver] characteristics_discover error invalid address\n");
  } else if (err_code == NRF_ERROR_BUSY) {
    PRINTF("[nest driver] characteristics_discover error busy\n");
  }

  return err_code;
}
/*---------------------------------------------------------------------------*/
static void
on_primary_srv_discovery_rsp(ble_db_discovery_t * const    p_db_discovery,
                             const ble_gattc_evt_t * const p_ble_gattc_evt)
{
  ret_code_t err_code;

  if (p_ble_gattc_evt->conn_handle != p_db_discovery->conn_handle) {
    return;
  }
  if (p_ble_gattc_evt->gatt_status == BLE_GATT_STATUS_SUCCESS)
  {
    const ble_gattc_evt_prim_srvc_disc_rsp_t * p_prim_srvc_disc_rsp_evt =
                              &(p_ble_gattc_evt->params.prim_srvc_disc_rsp);

    p_db_discovery->service.srv_uuid = p_prim_srvc_disc_rsp_evt->services[0].uuid;
    p_db_discovery->service.handle_range = p_prim_srvc_disc_rsp_evt->services[0].handle_range;

    err_code = characteristics_discover(p_db_discovery,
                                        p_ble_gattc_evt->conn_handle);
    if (err_code != NRF_SUCCESS){

      p_db_discovery->discovery_in_progress = false;

      // nest_service_discovered_event(p_ble_gattc_evt->conn_handle, false, 0);
      m_ble_service_discovery_params.conn_id = p_ble_gattc_evt->conn_handle;
      if (process_is_running(&nrf_service_discovery_process)) {
        process_post(&nrf_service_discovery_process, PROCESS_EVENT_POLL, NULL);
      }
    }
  }
  else
  {
    PRINTF("[nest driver] service UUID 0x%x Not found\n", p_db_discovery->service.srv_uuid.uuid);

    // No more service discovery is needed.
    p_db_discovery->discovery_in_progress  = false;

    // nest_service_discovered_event(p_ble_gattc_evt->conn_handle, false, 0);
    m_ble_service_discovery_params.conn_id = p_ble_gattc_evt->conn_handle;
    if (process_is_running(&nrf_service_discovery_process)) {
      process_post(&nrf_service_discovery_process, PROCESS_EVENT_POLL, NULL);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
on_characteristic_discovery_rsp(ble_db_discovery_t * const    p_db_discovery,
                                const ble_gattc_evt_t * const    p_ble_gattc_evt)
{
  uint32_t                 err_code;
  ble_gatt_db_srv_t      * p_srv_being_discovered;
  bool                     perform_desc_discov = false;

  if (p_ble_gattc_evt->conn_handle != p_db_discovery->conn_handle) {
    return;
  }

  p_srv_being_discovered = &(p_db_discovery->service);

  if (p_ble_gattc_evt->gatt_status == BLE_GATT_STATUS_SUCCESS)
  {
    const ble_gattc_evt_char_disc_rsp_t * p_char_disc_rsp_evt =
                                      &(p_ble_gattc_evt->params.char_disc_rsp);

    // Find out the number of characteristics that were previously discovered (in earlier
    // characteristic discovery responses, if any).
    uint8_t num_chars_prev_disc = p_srv_being_discovered->char_count;

    // Find out the number of characteristics that are currently discovered (in the
    // characteristic discovery response being handled).
    uint8_t num_chars_curr_disc = p_char_disc_rsp_evt->count;

    // Check if the total number of discovered characteristics are supported by this module.
    if ((num_chars_prev_disc + num_chars_curr_disc) <= BLE_GATT_DB_MAX_CHARS){
        // Update the characteristics count.
      p_srv_being_discovered->char_count += num_chars_curr_disc;
    } else {
      // The number of characteristics discovered at the peer is more than the supported
      // maximum. This module will store only the characteristics found up to this point.
      p_srv_being_discovered->char_count = BLE_GATT_DB_MAX_CHARS;
    }

    uint32_t i;
    uint32_t j;
    for (i = num_chars_prev_disc, j = 0; i < p_srv_being_discovered->char_count; i++, j++) {
      p_srv_being_discovered->charateristics[i].characteristic =
          p_char_disc_rsp_evt->chars[j];

      p_srv_being_discovered->charateristics[i].cccd_handle = BLE_GATT_HANDLE_INVALID;
    }

    ble_gattc_char_t * p_last_known_char =
            &(p_srv_being_discovered->charateristics[i - 1].characteristic);

    // If no more characteristic discovery is required, or if the maximum number of supported
    // characteristic per service has been reached, descriptor discovery will be performed.
    if ( !is_char_discovery_reqd(p_db_discovery, p_last_known_char) ||
        (p_srv_being_discovered->char_count == BLE_GATT_DB_MAX_CHARS))
    {
      perform_desc_discov = true;
    }
    else
    {
      // Update the current characteristic index.
      p_db_discovery->curr_char_ind = p_srv_being_discovered->char_count;

      // Perform another round of characteristic discovery.
      err_code = characteristics_discover(p_db_discovery,
                                          p_ble_gattc_evt->conn_handle);

      if (err_code != NRF_SUCCESS) {
        p_db_discovery->discovery_in_progress = false;

        // nest_service_discovered_event(p_ble_gattc_evt->conn_handle, false, 0);
        m_ble_service_discovery_params.conn_id = p_ble_gattc_evt->conn_handle;
        if (process_is_running(&nrf_service_discovery_process)) {
          process_post(&nrf_service_discovery_process, PROCESS_EVENT_POLL, NULL);
        }
        return;
      }
    }
  }
  else
  {
    // The previous characteristic discovery resulted in no characteristics.
    // descriptor discovery should be performed.
    perform_desc_discov = true;
  }

  if (perform_desc_discov)
  {
    bool raise_discov_complete;

    p_db_discovery->curr_char_ind = 0;

    err_code = descriptors_discover(p_db_discovery,
                                    &raise_discov_complete,
                                    p_ble_gattc_evt->conn_handle);

    if (err_code != NRF_SUCCESS) {
      // Error with discovering the service.
      // Indicate the error to the registered user application.
      p_db_discovery->discovery_in_progress = false;

      // nest_service_discovered_event(p_ble_gattc_evt->conn_handle, false, 0);
      m_ble_service_discovery_params.conn_id = p_ble_gattc_evt->conn_handle;
      if (process_is_running(&nrf_service_discovery_process)) {
        process_post(&nrf_service_discovery_process, PROCESS_EVENT_POLL, NULL);
      }
      return;
    }

    if (raise_discov_complete)
    {
      // No more characteristics and descriptors need to be discovered. Discovery is complete.
      // Send a discovery complete event to the user application.
      PRINTF("[nest driver] discovery of service with UUID 0x%x completed handle %d\n", p_srv_being_discovered->srv_uuid.uuid,
             p_ble_gattc_evt->conn_handle);

      p_db_discovery->discovery_in_progress = false;

      // nest_service_discovered_event(p_ble_gattc_evt->conn_handle, true, p_srv_being_discovered->char_count);
      m_ble_service_discovery_params.conn_id = p_ble_gattc_evt->conn_handle;
      m_ble_service_discovery_params.characteristic_count = p_srv_being_discovered->char_count;
      if (process_is_running(&nrf_service_discovery_process)) {
        process_post(&nrf_service_discovery_process, nrf_sd_event_services_discovered, NULL);
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
static void on_descriptor_discovery_rsp(ble_db_discovery_t * const    p_db_discovery,
                                        const ble_gattc_evt_t * const p_ble_gattc_evt)
{
  const ble_gattc_evt_desc_disc_rsp_t * p_desc_disc_rsp_evt;
  ble_gatt_db_srv_t                   * p_srv_being_discovered;

  if (p_ble_gattc_evt->conn_handle != p_db_discovery->conn_handle){
      return;
  }

  p_srv_being_discovered = &(p_db_discovery->service);
  p_desc_disc_rsp_evt = &(p_ble_gattc_evt->params.desc_disc_rsp);

  ble_gatt_db_char_t * p_char_being_discovered =
      &(p_srv_being_discovered->charateristics[p_db_discovery->curr_char_ind]);

  if (p_ble_gattc_evt->gatt_status == BLE_GATT_STATUS_SUCCESS) {
    // The descriptor was found at the peer.
    // If the descriptor was a CCCD, then the cccd_handle needs to be populated.
    uint32_t i;
    // Loop through all the descriptors to find the CCCD.
    for (i = 0; i < p_desc_disc_rsp_evt->count; i++)
    {
      if ( p_desc_disc_rsp_evt->descs[i].uuid.uuid ==
            BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG )
      {
        p_char_being_discovered->cccd_handle = p_desc_disc_rsp_evt->descs[i].handle;
        break;
      }
    }
  }

  bool raise_discov_complete = false;
  if ((p_db_discovery->curr_char_ind + 1) == p_srv_being_discovered->char_count)
  {
    // No more characteristics and descriptors need to be discovered. Discovery is complete.
    // Send a discovery complete event to the user application.
    raise_discov_complete = true;
  }
  else
  {
    // Begin discovery of descriptors for the next characteristic.
    uint32_t err_code;

    p_db_discovery->curr_char_ind++;

    err_code = descriptors_discover(p_db_discovery,
                                    &raise_discov_complete,
                                    p_ble_gattc_evt->conn_handle);
    if (err_code != NRF_SUCCESS) {
      // Error with discovering the service.
      // Indicate the error to the registered user application.
      p_db_discovery->discovery_in_progress = false;
      return;
    }
  }

  if (raise_discov_complete) {
    PRINTF("[nest driver] service discovery UUID 0x%x success conn id %d\n", p_srv_being_discovered->srv_uuid.uuid,
           p_ble_gattc_evt->conn_handle);

    p_db_discovery->discovery_in_progress = false;

    // nest_service_discovered_event(p_ble_gattc_evt->conn_handle, true, p_srv_being_discovered->char_count);
    m_ble_service_discovery_params.conn_id = p_ble_gattc_evt->conn_handle;
    m_ble_service_discovery_params.characteristic_count = p_srv_being_discovered->char_count;
    if (process_is_running(&nrf_service_discovery_process)) {
      process_post(&nrf_service_discovery_process, nrf_sd_event_services_discovered, NULL);
    }
  }
}
/*---------------------------------------------------------------------------*/
static void
on_ble_central_discovert_evt(ble_db_discovery_t * const p_db_discovery, const ble_evt_t * const p_ble_evt)
{
  const ble_gap_evt_t * const p_gap_evt = &p_ble_evt->evt.gap_evt;

  switch (p_ble_evt->header.evt_id)
  {
    case BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP:
    {
      on_primary_srv_discovery_rsp(p_db_discovery, &(p_ble_evt->evt.gattc_evt));
    }
    break; // BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP
    case BLE_GATTC_EVT_CHAR_DISC_RSP:
    {
      on_characteristic_discovery_rsp(p_db_discovery, &(p_ble_evt->evt.gattc_evt));
    }
    break; // BLE_GATTC_EVT_CHAR_DISC_RSP
    case BLE_GATTC_EVT_DESC_DISC_RSP:
    {
      on_descriptor_discovery_rsp(p_db_discovery, &(p_ble_evt->evt.gattc_evt));
    }
    break; // BLE_GATTC_EVT_DESC_DISC_RSP
    default:
    break; // default
  }
}
/*---------------------------------------------------------------------------*/
static void
on_ble_central_evt(const ble_evt_t * const p_ble_evt)
{
  const ble_gap_evt_t   * const p_gap_evt = &p_ble_evt->evt.gap_evt;
  const ble_gap_evt_connected_t * const p_evt_connect = &(p_gap_evt->params.connected);
  static nrf_gap_connected_event_t     connected_event_params;
  static nrf_gap_disconnect_event_t    disconnect_event_params;
  static nrf_gatt_write_value_event_t  gattc_write_event_params;
  ret_code_t                    err_code;
  nest_scanner_peer_t           report;

  switch (p_ble_evt->header.evt_id)
  {
    case BLE_GAP_EVT_CONNECTED:
    {
      // PRINTF("[nest driver] on ble central event: connected connId %d\n",
      //                      p_gap_evt->conn_handle);
      if (!(p_gap_evt->conn_handle < NEST_CENTRAL_LINK_COUNT + NEST_PERIPHERAL_LINK_COUNT)) {
        PRINTF("[nest driver] Exceed the link limitation\n");
        return;
      }
      // peer_type = p_evt_connect->peer_addr.addr_type;
      // memcpy(peer_address, p_evt_connect->peer_addr.addr, 6);
      // nest_connected_event(p_gap_evt->conn_handle, NEST_BLE_ROLE_CENTRAL,
      //                            peer_address, peer_type);
      // nrf_gap_central_connected_event(p_gap_evt->conn_handle, peer_address, peer_type);
      connected_event_params.conn_id = p_gap_evt->conn_handle;
      memcpy(connected_event_params.address, p_evt_connect->peer_addr.addr, 6);
      connected_event_params.type = p_evt_connect->peer_addr.addr_type;
      connected_event_params.role = NEST_BLE_ROLE_CENTRAL;
      process_post(&nrf_sd_event_process, nrf_sd_event_gap_connected, (void *)&connected_event_params);
    }
    break; // BLE_GAP_EVT_CONNECTED
    case BLE_GAP_EVT_DISCONNECTED:
    {
      // PRINTF("[nest driver] on ble central event: disconnected connId %d reason %d\n",
      //                      p_ble_evt->evt.gap_evt.conn_handle,
      //                      p_ble_evt->evt.gap_evt.params.disconnected.reason);

      // clear the service discovery content
      ble_db_discovery_t  *p_db_discovery = &m_ble_db_discovery[p_ble_evt->evt.gap_evt.conn_handle];
      p_db_discovery->discovery_in_progress = false;
      memset(&p_db_discovery->service, 0x00, (sizeof(ble_gatt_db_srv_t) ) );
      // nest_disconnect_event(p_ble_evt->evt.gap_evt.conn_handle,
      //                       p_ble_evt->evt.gap_evt.params.disconnected.reason);
      // nrf_gap_central_disconnect_event(p_ble_evt->evt.gap_evt.conn_handle,
      //                       p_ble_evt->evt.gap_evt.params.disconnected.reason);
      disconnect_event_params.conn_id = p_ble_evt->evt.gap_evt.conn_handle;
      disconnect_event_params.reason = p_ble_evt->evt.gap_evt.params.disconnected.reason;
      process_post(&nrf_sd_event_process, nrf_sd_event_gap_disconnect, (void *)&disconnect_event_params);
    }
    break; // BLE_GAP_EVT_DISCONNECTED
    case BLE_GATTC_EVT_HVX:
    {
      // PRINTF("[nest driver] gattc gvx connId %d\n", p_gap_evt->conn_handle);
      // HVX can only occur from client sending.
      // nest_gattc_handle_value_event(p_gap_evt->conn_handle,
      //                         p_ble_evt->evt.gattc_evt.params.hvx.handle,
      //                         // (uint8_t *)p_ble_evt->evt.gattc_evt.params.hvx.data,
      //                         &p_ble_evt->evt.gattc_evt.params.hvx.data[0],
      //                         p_ble_evt->evt.gattc_evt.params.hvx.len);
      // nrf_gattc_handle_value_event(p_gap_evt->conn_handle,
      //                         p_ble_evt->evt.gattc_evt.params.hvx.handle,
      //                         // (uint8_t *)p_ble_evt->evt.gattc_evt.params.hvx.data,
      //                         &p_ble_evt->evt.gattc_evt.params.hvx.data[0],
      //                         p_ble_evt->evt.gattc_evt.params.hvx.len);
      gattc_write_event_params.conn_id = p_gap_evt->conn_handle;
      gattc_write_event_params.handle = p_ble_evt->evt.gattc_evt.params.hvx.handle;
      memcpy(gattc_write_event_params.data, &p_ble_evt->evt.gattc_evt.params.hvx.data[0], p_ble_evt->evt.gattc_evt.params.hvx.len);
      gattc_write_event_params.length = p_ble_evt->evt.gattc_evt.params.hvx.len;
      process_post(&nrf_sd_event_process, nrf_sd_event_gattc_hvx, (void *)&gattc_write_event_params);
    }
    break; // BLE_GATTC_EVT_HVX
    case BLE_GATTC_EVT_WRITE_RSP:
    {
      // TODO:
    }
    break; // BLE_GATTC_EVT_WRITE_RSP
    case BLE_GAP_EVT_ADV_REPORT:
    {
      // Initialize advertisement report for parsing.
      if (p_gap_evt->params.adv_report.rssi == 0) {
        return;
      }
      // PRINTF("[nest driver] scanned device: id_peer:%d type:%d %02x:%02x:%02x:%02x:%02x:%02x rssi: %d\n",
      //         p_gap_evt->params.adv_report.peer_addr.addr_id_peer,
      //         p_gap_evt->params.adv_report.peer_addr.addr_type,
      //         p_gap_evt->params.adv_report.peer_addr.addr[0], p_gap_evt->params.adv_report.peer_addr.addr[1],
      //         p_gap_evt->params.adv_report.peer_addr.addr[2], p_gap_evt->params.adv_report.peer_addr.addr[3],
      //         p_gap_evt->params.adv_report.peer_addr.addr[4], p_gap_evt->params.adv_report.peer_addr.addr[5],
      //         p_gap_evt->params.adv_report.rssi );
      // TODO: queue and post evnet
      report.peer_addr_type = p_gap_evt->params.adv_report.peer_addr.addr_type;
      memcpy(report.peer_address , p_gap_evt->params.adv_report.peer_addr.addr, 6);
      memcpy(report.data , p_gap_evt->params.adv_report.data, ADV_MAX_DATA_SIZE);
      report.rssi = p_gap_evt->params.adv_report.rssi;
      report.len = p_gap_evt->params.adv_report.dlen;
      report.scan_response = p_gap_evt->params.adv_report.scan_rsp;
      report.type = p_gap_evt->params.adv_report.type;
      scq_enqueue(&report, INPUT_ALLOC);
      process_post(&nrf_sd_event_process, nrf_sd_event_adv_report, NULL);
      // nest_scanner_result_event(&report);
    }
    break; // BLE_GAP_ADV_REPORT
    case BLE_GAP_EVT_TIMEOUT:
    {
      // We have not specified a timeout for scanning, so only connection attemps can timeout.
      if (p_gap_evt->params.timeout.src == BLE_GAP_TIMEOUT_SRC_CONN) {
        process_post(&nrf_sd_event_process, nrf_sd_event_gap_connect_timeout, NULL);
      }
    }
    break; // BLE_GAP_EVT_TIMEOUT
    case BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST:
    {
      // Accept parameters requested by peer.
      err_code = sd_ble_gap_conn_param_update(p_gap_evt->conn_handle,
                             &p_gap_evt->params.conn_param_update_request.conn_params);
      PRINTF("[nest driver] update connection parameters\n");

      if (err_code != NRF_SUCCESS) {
        PRINTF("[nest driver] sd_ble_gap_conn_param_update error:%d\n", err_code);
      }
    }
    break; // BLE_GAP_EVT_CONN_PARAM_UPDATE_REQUEST
    case BLE_GATTC_EVT_TIMEOUT:
    {
      // Disconnect on GATT Client timeout event.
      PRINTF("[nest driver] GATT Client Timeout.\n");
      err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                       BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
      APP_ERROR_CHECK(err_code);
    }
    break; // BLE_GATTC_EVT_TIMEOUT
    case BLE_GATTS_EVT_TIMEOUT:
    {
      // Disconnect on GATT Server timeout event.
      PRINTF("[nest driver] GATT Server Timeout.\n");
      err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                       BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
      APP_ERROR_CHECK(err_code);
    }
    break; // BLE_GATTS_EVT_TIMEOUT
#if NRF_SD_BLE_API_VERSION == 3
    case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
    {
      err_code = sd_ble_gatts_exchange_mtu_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                 NRF_BLE_MAX_MTU_SIZE);
      APP_ERROR_CHECK(err_code);
    }
    break; // BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST
#endif
    default:
    // No implementation needed.
    break;
  }
}
/*---------------------------------------------------------------------------*/
static void
on_ble_peripheral_evt(ble_evt_t * p_ble_evt)
{
  const ble_gap_evt_t   * const p_gap_evt = &p_ble_evt->evt.gap_evt;
  const ble_gap_evt_connected_t * const p_evt_connect = &(p_gap_evt->params.connected);
  static nrf_gap_connected_event_t connected_event_params;
  static nrf_gap_disconnect_event_t disconnect_event_params;
  static nrf_gatt_write_value_event_t  gatts_write_event_params;
  ret_code_t err_code;
  // uint8_t peer_address[6];
  // uint8_t peer_type;

  switch (p_ble_evt->header.evt_id)
  {
    case BLE_GAP_EVT_CONNECTED:
    {
      PRINTF("[nest driver] on ble peripheral event: connected connId %d\n",
                             p_gap_evt->conn_handle);
      // peer_type = p_evt_connect->peer_addr.addr_type;
      // memcpy(peer_address, p_evt_connect->peer_addr.addr, 6);
      // nest_connected_event(p_gap_evt->conn_handle, NEST_BLE_ROLE_PERIPHERAL,
      //                                                  peer_address, peer_type);
      connected_event_params.conn_id = p_gap_evt->conn_handle;
      memcpy(connected_event_params.address, p_evt_connect->peer_addr.addr, 6);
      connected_event_params.type = p_evt_connect->peer_addr.addr_type;
      connected_event_params.role = NEST_BLE_ROLE_PERIPHERAL;
      process_post(&nrf_sd_event_process, nrf_sd_event_gap_connected, (void *)&connected_event_params);
    }
    break; //BLE_GAP_EVT_CONNECTED
    case BLE_GAP_EVT_DISCONNECTED:
    {
      //PRINTF("[nest driver] on ble peripheral event: disconnected connId %d reason %d\n",
      //                        p_ble_evt->evt.gap_evt.conn_handle,
      //                        p_ble_evt->evt.gap_evt.params.disconnected.reason);
      // nest_disconnect_event(p_ble_evt->evt.gap_evt.conn_handle,
      //                       p_ble_evt->evt.gap_evt.params.disconnected.reason);
      disconnect_event_params.conn_id = p_ble_evt->evt.gap_evt.conn_handle;
      disconnect_event_params.reason = p_ble_evt->evt.gap_evt.params.disconnected.reason;
      process_post(&nrf_sd_event_process, nrf_sd_event_gap_disconnect, (void *)&disconnect_event_params);
    }
    break;//BLE_GAP_EVT_DISCONNECTED
    case BLE_GATTC_EVT_TIMEOUT:
    {
      // Disconnect on GATT Client timeout event.
      // PRINTF("[nest driver] GATT Client Timeout.\r\n");
      err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                       BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
      APP_ERROR_CHECK(err_code);
    }
    break; // BLE_GATTC_EVT_TIMEOUT
    case BLE_GATTS_EVT_TIMEOUT:
    {
      // Disconnect on GATT Server timeout event.
      // PRINTF("[nest driver] GATT Server Timeout.\r\n");
      err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                       BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
      APP_ERROR_CHECK(err_code);
    }
    break; // BLE_GATTS_EVT_TIMEOUT
    case BLE_EVT_USER_MEM_REQUEST:
    {
      err_code = sd_ble_user_mem_reply(p_ble_evt->evt.gap_evt.conn_handle, NULL);
      APP_ERROR_CHECK(err_code);
    }
    break;//BLE_EVT_USER_MEM_REQUEST
    case BLE_GATTS_EVT_SYS_ATTR_MISSING:
    {
      // No system attributes have been stored.
      err_code = sd_ble_gatts_sys_attr_set(p_ble_evt->evt.gap_evt.conn_handle, NULL, 0, 0);
      if (err_code != NRF_SUCCESS) {
        PRINTF("[nest driver] sd_ble_gatts_sys_attr_set error:%d\n", err_code);
      }
    }
    break;//BLE_GATTS_EVT_SYS_ATTR_MISSING
    case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
		{
			// Pairing not supported
      err_code = sd_ble_gap_sec_params_reply(p_ble_evt->evt.gap_evt.conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, NULL, NULL);
			if (err_code != NRF_SUCCESS) {
				PRINTF("[nest driver] sd_ble_gap_sec_params_reply error:%d\n", err_code);
			}
		}
    break;
    case BLE_GATTS_EVT_WRITE:
    {
      PRINTF("[nest driver] gatts write event: write handle %d\n",
                                p_ble_evt->evt.gatts_evt.params.write.handle);
      // nest_gatts_write_event(p_ble_evt->evt.gap_evt.conn_handle,
      //                           p_ble_evt->evt.gatts_evt.params.write.handle,
      //                           p_ble_evt->evt.gatts_evt.params.write.data,
      //                           p_ble_evt->evt.gatts_evt.params.write.len);
      gatts_write_event_params.conn_id = p_ble_evt->evt.gap_evt.conn_handle;
      gatts_write_event_params.handle = p_ble_evt->evt.gatts_evt.params.write.handle;
      memcpy(gatts_write_event_params.data, p_ble_evt->evt.gatts_evt.params.write.data,
        p_ble_evt->evt.gatts_evt.params.write.len);
      gatts_write_event_params.length = p_ble_evt->evt.gatts_evt.params.write.len;
      process_post(&nrf_sd_event_process, nrf_sd_event_gatts_write, (void *)&gatts_write_event_params);
		}
		break; // BLE_GATTS_EVT_WRITE
    case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
    {
      ble_gatts_evt_rw_authorize_request_t  req;
      ble_gatts_rw_authorize_reply_params_t auth_reply;
      req = p_ble_evt->evt.gatts_evt.params.authorize_request;
      if (req.type != BLE_GATTS_AUTHORIZE_TYPE_INVALID)
      {
        if ((req.request.write.op == BLE_GATTS_OP_PREP_WRITE_REQ)     ||
            (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_NOW) ||
            (req.request.write.op == BLE_GATTS_OP_EXEC_WRITE_REQ_CANCEL))
        {
          if (req.type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
          {
              auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
          }
          else
          {
              auth_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
          }
          auth_reply.params.write.gatt_status = APP_FEATURE_NOT_SUPPORTED;
          err_code = sd_ble_gatts_rw_authorize_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                     &auth_reply);
          APP_ERROR_CHECK(err_code);
        }
      }
    }
    break; // BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST
#if NRF_SD_BLE_API_VERSION == 3
    case BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST:
    {
      err_code = sd_ble_gatts_exchange_mtu_reply(p_ble_evt->evt.gatts_evt.conn_handle,
                                                 NRF_BLE_MAX_MTU_SIZE);
      APP_ERROR_CHECK(err_code);
    }
    break; // BLE_GATTS_EVT_EXCHANGE_MTU_REQUEST
#endif
    default:
    // No implementation needed.
    break;
  }
}
/*---------------------------------------------------------------------------*/
static void
ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
  const ble_gap_evt_t   *p_gap_evt = &p_ble_evt->evt.gap_evt;
	ret_code_t err_code;
  uint16_t conn_handle;
  uint16_t role;

  ble_conn_state_on_ble_evt(p_ble_evt);

  // The connection handle should really be retrievable for any event type.
  conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
  role        = ble_conn_state_role(conn_handle);

//  PRINTF("[nest driver] event evt:%d\n", p_ble_evt->header.evt_id);

  // Based on the role this device plays in the connection, dispatch to the right applications.
  if (role == BLE_GAP_ROLE_PERIPH)
  {
    on_ble_peripheral_evt(p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
  }
  else if ((role == BLE_GAP_ROLE_CENTRAL) || (p_ble_evt->header.evt_id == BLE_GAP_EVT_ADV_REPORT))
  {
    /** on_ble_central_evt will update the connection handles, so we want to execute it
     * after dispatching to the central applications upon disconnection. */
    if (p_ble_evt->header.evt_id != BLE_GAP_EVT_DISCONNECTED) {
      on_ble_central_evt(p_ble_evt);
    }

    if (conn_handle < NEST_CENTRAL_LINK_COUNT + NEST_PERIPHERAL_LINK_COUNT) {
      on_ble_central_discovert_evt(&m_ble_db_discovery[conn_handle], p_ble_evt);
    }

    // If the peer disconnected, we update the connection handles last.
    if (p_ble_evt->header.evt_id == BLE_GAP_EVT_DISCONNECTED) {
      on_ble_central_evt(p_ble_evt);
    }
  }
  else
  {
    if (p_ble_evt->header.evt_id == BLE_GAP_EVT_CONNECTED) {
      PRINTF("[nest driver] connected but role mix %d\n", role);
    }
  }

  if (p_ble_evt->header.evt_id == BLE_EVT_TX_COMPLETE) {
    // TODO tx completed
    process_post(&nrf_sd_event_process, nrf_sd_event_tx_completed, NULL);
    // nest_tx_completed();
    // nrf_gattc_write_tx_complete();
  }
}
/*---------------------------------------------------------------------------*/
static int
nrf_gatt_start_service_discovery(uint8_t conn_id, nest_bleuuid_t *p_nest_uuid, nest_service_discovery_callback_t *callback)
{
  // start to discovery service
  ble_db_discovery_t  *p_db_discovery = &m_ble_db_discovery[conn_id];
  ret_code_t err_code = NRF_SUCCESS;

  if (p_nest_uuid == NULL || callback == NULL) {
    return ENULLP;
  }

  if (callback->completed == NULL || callback->abort == NULL) {
    return ENULLP;
  }

  if ( (p_db_discovery->discovery_in_progress) ||
       (process_is_running(&nrf_service_discovery_process)) )
  {
    // Error Busy
    PRINTF("[nest driver] start to discovery error, busy\n");
    return EBUSY;
  }

  p_db_discovery->conn_handle = conn_id;
  p_db_discovery->curr_char_ind = 0;
  p_db_discovery->service.srv_uuid.uuid = p_nest_uuid->uuid;
  p_db_discovery->service.srv_uuid.type = p_nest_uuid->type;

  m_service_discovery_callback.completed = callback->completed;
  m_service_discovery_callback.abort = callback->abort;

  PRINTF("[nest driver] starting discovery uuid:0x%04x tpye %d, connId %d\n", p_nest_uuid->uuid, p_nest_uuid->type, conn_id);

  if (!process_is_running(&nrf_service_discovery_process))
    process_start(&nrf_service_discovery_process, NULL);

  // if all primary services will be returned.
  err_code = sd_ble_gattc_primary_services_discover(conn_id,
                                                    SRV_DISC_START_HANDLE,
                                                    &(p_db_discovery->service.srv_uuid));
  if (err_code != NRF_SUCCESS) {
    m_ble_service_discovery_params.conn_id = conn_id;
    if (process_is_running(&nrf_service_discovery_process)) {
      process_post(&nrf_service_discovery_process, PROCESS_EVENT_POLL, NULL);
    }
    if (err_code == BLE_ERROR_INVALID_CONN_HANDLE) {
      PRINTF("[nest driver] discovery invalid conn handle\n");
      return EINVAL;
    } else if (err_code == NRF_ERROR_INVALID_STATE) {
      PRINTF("[nest driver] discovery invalid state\n");
      return EINVALSTATE;
    } else if (err_code == NRF_ERROR_INVALID_PARAM) {
      PRINTF("[nest driver] discovery invalid param\n");
      return EINVAL;
    } else if (err_code == NRF_ERROR_BUSY) {
      PRINTF("[nest driver] discovery busy\n");
      return EBUSY;
    }
  }

  p_db_discovery->discovery_in_progress = true;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
nrf_gattc_add_service(const nest_bleservice_t *service)
{
  uint32_t err_code;
  ble_uuid128_t vendor_uuid;

  if (service == NULL) {
    return ENULLP;
  }

  if (service->bleuuid.type == NEST_BLE_UUID_TYPE_VENDOR ) {
    memcpy(&vendor_uuid.uuid128[0], &service->vendor_uuid.uuid128[0], 16);
    err_code = sd_ble_uuid_vs_add(&(vendor_uuid), &(service->bleuuid.type));
    if (err_code == NRF_ERROR_INVALID_ADDR) {
      return EINVAL;
    } else if (err_code == NRF_ERROR_NO_MEM) {
      return ENOMEM;
    }
  }
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
nrf_gattc_get_service(uint16_t conn_id, nest_bleuuid_t *p_srv_uuid,
                  uint8_t char_idx, nest_blecharacteristic_t *p_characteristic)
{
  ble_db_discovery_t  *p_db_discovery = &m_ble_db_discovery[conn_id];
  ble_gatt_db_srv_t   *p_srv_being_discovered;
  ble_gatt_db_char_t  *p_chars;
  uint32_t i = char_idx;
//  PRINTF("[nest driver] gattc get service uuid 0x%04x type %d\n",
//                                            p_srv_uuid->uuid, p_srv_uuid->type);

  if (p_db_discovery->discovery_in_progress) {
    PRINTF("[nest driver] gattc get service error, discovery in progress\n");
    return EINVALSTATE;
  }

  p_srv_being_discovered = &(p_db_discovery->service);
  p_chars = p_srv_being_discovered->charateristics;

  if (i >= p_srv_being_discovered->char_count) {
    PRINTF("[nest driver] gattc get service error, invalid index\n");
    return EINVAL;
  }

  PRINTF("[nest driver] characteristic count %d\n", p_srv_being_discovered->char_count);

  if (p_srv_being_discovered->srv_uuid.uuid == p_srv_uuid->uuid &&
      p_srv_being_discovered->srv_uuid.type == p_srv_uuid->type)
  {
    p_characteristic->value_handle = p_chars[i].characteristic.handle_value;
    p_characteristic->uuid = p_chars[i].characteristic.uuid.uuid;
    p_characteristic->props.broadcast = p_chars[i].characteristic.char_props.broadcast;
    p_characteristic->props.read = p_chars[i].characteristic.char_props.read;
    p_characteristic->props.write_wo_resp = p_chars[i].characteristic.char_props.write_wo_resp;
    p_characteristic->props.write = p_chars[i].characteristic.char_props.write;
    p_characteristic->props.notify = p_chars[i].characteristic.char_props.notify;
    p_characteristic->props.indicate = p_chars[i].characteristic.char_props.indicate;
    p_characteristic->props.auth_signed_wr = p_chars[i].characteristic.char_props.auth_signed_wr;
    p_characteristic->props.char_ext_props = p_chars[i].characteristic.char_ext_props;
    p_characteristic->cccd_handle = p_chars[i].cccd_handle;
    // TODO: descriptors handle not setup for current patch
    //p_chars[i].characteristic.handle_decl;
  }
  else
  {
    return EINVAL;
  }
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
nrf_ble_reset(void)
{
  uint32_t err_code;

  softdevice_handler_suspend();
  err_code = softdevice_handler_sd_disable();
  if (err_code != NRF_SUCCESS) {
    PRINTF("[nest driver] sd_softdevice_disable error\n", err_code);
  } else {
    PRINTF("[nest driver] sd_softdevice_disable success\n");
  }
  softdevice_init();
  softdevice_handler_resume();

  // must initialized again
  err_code = ble_radio_notification_init(6,
                                         NRF_RADIO_NOTIFICATION_DISTANCE_800US,
                                         ble_on_radio_active_evt);
  if (err_code != NRF_SUCCESS){
    PRINTF("[nest driver] ble_radio_notification_init \n");
    return EINVAL;
  }

  nrf_gatts_init_characteristic_pool();
  nrf_gatts_init_service_pool();
  nest_bleservice_init();
  memset(m_ble_db_discovery, 0x00, (sizeof(ble_db_discovery_t) * (NEST_CENTRAL_LINK_COUNT + NEST_PERIPHERAL_LINK_COUNT)) );

  return ENONE;
}
/*---------------------------------------------------------------------------*/
static int
init(void)
{
	uint32_t err_code;

  memset(scanbuf, 0x00, sizeof(nest_scanner_peer_t) * SCAN_QUEUE_SIZE);
  memset(&scq, 0x00, sizeof(scq_t));

  nrf_sd_event_gap_connected = process_alloc_event();
  nrf_sd_event_gap_disconnect = process_alloc_event();
  nrf_sd_event_gattc_hvx = process_alloc_event();
  nrf_sd_event_gatts_write = process_alloc_event();
  nrf_sd_event_gap_connect_timeout = process_alloc_event();
  nrf_sd_event_tx_completed = process_alloc_event();
  nrf_sd_event_services_discovered = process_alloc_event();
  nrf_sd_event_adv_report = process_alloc_event();

  // start the process to handle the interrupt event
  if (!process_is_running(&nrf_sd_event_process))
    process_start(&nrf_sd_event_process, NULL);

	// Register with the SoftDevice handler module for BLE events.
	err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
	if (err_code != NRF_SUCCESS){
		PRINTF("Register the SoftDevice handler error %d\n", err_code);
		return EINVAL;
	}

  err_code = ble_radio_notification_init(6,
                                         NRF_RADIO_NOTIFICATION_DISTANCE_800US,
                                         ble_on_radio_active_evt);
  if (err_code != NRF_SUCCESS){
    PRINTF("[nest driver] ble_radio_notification_init \n");
    return EINVAL;
  }

  memset(m_ble_db_discovery, 0x00, (sizeof(ble_db_discovery_t) * (NEST_CENTRAL_LINK_COUNT + NEST_PERIPHERAL_LINK_COUNT)) );

  gap_params_init();
  nest_bleservice_init();
  conn_params_init();
  ble_conn_state_init();
  nrf_gatts_init_characteristic_pool();
  nrf_gatts_init_service_pool();

  PRINTF("[nest driver] initialized\n");
  return ENONE;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_driver_process, ev, data)
{
  static uint16_t event_type;
  PROCESS_BEGIN();
	while(1) {
    PROCESS_WAIT_EVENT_UNTIL( (ev == PROCESS_EVENT_POLL) );
    event_type = data;
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
const struct nest_driver nrf_nest_driver = {
	.name = "Nest driver",
	.init = init,
  .reset = nrf_ble_reset,
	.gap_broadcasting = nrf_gap_broadcasting,
  .gap_set_advdata = nrf_gap_set_adv_data,
	.gap_scan = nrf_gap_scanner,
	.gap_connect = nrf_gap_connect,
	.gap_disconnect = nrf_gap_disconnect,
	.gap_local_addr = nrf_getaddr,
  .gap_set_device_name = nrf_gap_set_device_name,
  .gap_set_ppcp = nrf_gap_set_ppcp,
  .gap_get_ppcp = nrf_gap_get_ppcp,
  .gap_update_ppcp = nrf_gap_update_ppcp,
  .gap_set_txpower = nrf_gap_set_txpower,
  .gatt_start_service_discovery = nrf_gatt_start_service_discovery,
  .gatts_add_service = nrf_gatts_add_service,
  .gatts_add_characteristic = nrf_gatts_add_characteristic,
  .gatts_handle_value = nrf_gatts_handle_value,
  .gattc_add_service = nrf_gattc_add_service,
  .gattc_get_service = nrf_gattc_get_service,
  .gattc_write = nrf_gattc_write
};
/*---------------------------------------------------------------------------*/
int
is_radio_accessing(void)
{
  return m_radio_active;
}
/*---------------------------------------------------------------------------*/
