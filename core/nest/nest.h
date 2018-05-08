/**
 *  Copyright (c) 2016 AlfaLoop Technology Co., Ltd. All Rights Reserved.
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
#ifndef _NEST_H__
#define _NEST_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "contiki.h"
#include <stdint.h>
#include <stdbool.h>

#include "errno.h"

/*---------------------------------------------------------------------------*/
#ifndef NEST_CONF_DRIVER
#define NEST_CONF_DRIVER nullNEST_driver
#else
#define NEST NEST_CONF_DRIVER
#endif /* NEST_CONF_DRIVER */

/*---------------------------------------------------------------------------*/
#define NEST_PROTOCOL_VERSION 		 			 0x03
#define NEST_SYSTEM_VERSION_H						 NEST_PLATFORM_VERSION_H_CONF
#define NEST_SYSTEM_VERSION_L						 NEST_PLATFORM_VERSION_L_CONF
#define NEST_ADV_TYPE_NFD_RSSI			 		 NEST_ADV_TYPE_NFD_RSSI_CONF
#define NEST_ADD_GATTS_SERVICE			 		 NEST_ADD_GATTS_SERVICE_CONF
#define NEST_ADD_APP_GATTS_SERVICE			 NEST_ADD_GATTS_SERVICE - 1

#define NEST_SCAN_INTERVAL               0x00A0                                        /**< 100ms Determines scan interval in units of 0.625 millisecond. */
#define NEST_SCAN_WINDOW                 0x0050                                        /**< 50ms Determines scan window in units of 0.625 millisecond. */
#define NEST_SCAN_TIMEOUT                3

#define NEST_MIN_CONNECTION_INTERVAL     10   										/**< Determines minimum connection interval in milliseconds. */
#define NEST_MAX_CONNECTION_INTERVAL     60    								  /**< Determines maximum connection interval in milliseconds. */
#define NEST_SLAVE_LATENCY               0                        /**< Determines slave latency in terms of connection events. */
#define NEST_SUPERVISION_TIMEOUT         4000

#define NEST_CENTRAL_USER_LINK_COUNT      NEST_CENTRAL_USER_LINK_COUNT_CONF
#define NEST_CENTRAL_LINK_COUNT          	NEST_CENTRAL_USER_LINK_COUNT + 1
#define NEST_PERIPHERAL_LINK_COUNT       	NEST_PERIPHERAL_LINK_COUNT_CONF
#define NEST_TOTAL_CHANNEL								NEST_CENTRAL_LINK_COUNT + NEST_PERIPHERAL_LINK_COUNT

#define NEST_BLE_DISCOVERY_MAX_SRV				6
#define BLE_DEVICE_ADDR_LEN								6
#define BLE_CCCD_HANDLE_VALUE_LEN					2
#define BLE_UUID_NEST_VENDOR_128UUID			     {0x09, 0x2B, 0xE4, 0xA1, 0x95, 0x82, 0x00, 0x17, 0x94, 0xFE, 0x7C, 0x34, 0x00, 0x17, 0x73, 0xAF}
#define BLE_UUID_NEST_SERVICE             			0x1700                      /**< The UUID of the NCS ervice. */
#define BLE_UUID_NEST_INBOUND_CHARACTERISTIC  	0x1701                      /**< The UUID of the Inbound transmission Characteristic. */
#define BLE_UUID_NEST_OUTBOUND_CHARACTERISTIC   0x1702                      /**< The UUID of the Outbounds transmission Characteristic. */
#define BLE_NEST_FIX_DATA_LEN										20

#define ADV_MAX_DATA_SIZE           31				// Maximum size of advertising data in octets.
#define ADV_TYPE_ADV_IND          	0x00  	  // Connectable undirected.
#define ADV_TYPE_ADV_DIRECT_IND   	0x01   		// Connectable directed.
#define ADV_TYPE_ADV_SCAN_IND     	0x02   		// Scannable undirected.
#define ADV_TYPE_ADV_NONCONN_IND  	0x03   		// Non connectable undirected.

#define GAP_ADDR_TYPE_PUBLIC                        0x00 /**< Public address. */
#define GAP_ADDR_TYPE_RANDOM_STATIC                 0x01 /**< Random static address. */
#define GAP_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE     0x02 /**< Random private resolvable address. */
#define GAP_ADDR_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE 0x03 /**< Random private non-resolvable address. */

/**@defgroup BLE_GAP_ADV_FLAGS GAP Advertisement Flags
 * @{ */
#define BLE_ADV_FLAG_LE_LIMITED_DISC_MODE         (0x01)   /**< LE Limited Discoverable Mode. */
#define BLE_ADV_FLAG_LE_GENERAL_DISC_MODE         (0x02)   /**< LE General Discoverable Mode. */
#define BLE_ADV_FLAG_BR_EDR_NOT_SUPPORTED         (0x04)   /**< BR/EDR not supported. */
#define BLE_ADV_FLAG_LE_BR_EDR_CONTROLLER         (0x08)   /**< Simultaneous LE and BR/EDR, Controller. */
#define BLE_ADV_FLAG_LE_BR_EDR_HOST               (0x10)   /**< Simultaneous LE and BR/EDR, Host. */
#define BLE_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE   (BLE_ADV_FLAG_LE_LIMITED_DISC_MODE | BLE_ADV_FLAG_BR_EDR_NOT_SUPPORTED)   /**< LE Limited Discoverable Mode, BR/EDR not supported. */
#define BLE_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE   (BLE_ADV_FLAG_LE_GENERAL_DISC_MODE | BLE_ADV_FLAG_BR_EDR_NOT_SUPPORTED)   /**< LE General Discoverable Mode, BR/EDR not supported. */

#define ADV_TYPE_FLAGS                               0x01 /**< Flags for discoverability. */
#define ADV_TYPE_16BIT_SERVICE_UUID_MORE_AVAILABLE   0x02 /**< Partial list of 16 bit service UUIDs. */
#define ADV_TYPE_16BIT_SERVICE_UUID_COMPLETE         0x03 /**< Complete list of 16 bit service UUIDs. */
#define ADV_TYPE_32BIT_SERVICE_UUID_MORE_AVAILABLE   0x04 /**< Partial list of 32 bit service UUIDs. */
#define ADV_TYPE_32BIT_SERVICE_UUID_COMPLETE         0x05 /**< Complete list of 32 bit service UUIDs. */
#define ADV_TYPE_128BIT_SERVICE_UUID_MORE_AVAILABLE  0x06 /**< Partial list of 128 bit service UUIDs. */
#define ADV_TYPE_128BIT_SERVICE_UUID_COMPLETE        0x07 /**< Complete list of 128 bit service UUIDs. */
#define ADV_TYPE_SHORT_LOCAL_NAME                    0x08 /**< Short local device name. */
#define ADV_TYPE_COMPLETE_LOCAL_NAME                 0x09 /**< Complete local device name. */
#define ADV_TYPE_TX_POWER_LEVEL                      0x0A /**< Transmit power level. */
#define ADV_TYPE_CLASS_OF_DEVICE                     0x0D /**< Class of device. */
#define ADV_TYPE_SIMPLE_PAIRING_HASH_C               0x0E /**< Simple Pairing Hash C. */
#define ADV_TYPE_SIMPLE_PAIRING_RANDOMIZER_R         0x0F /**< Simple Pairing Randomizer R. */
#define ADV_TYPE_SECURITY_MANAGER_TK_VALUE           0x10 /**< Security Manager TK Value. */
#define ADV_TYPE_SECURITY_MANAGER_OOB_FLAGS          0x11 /**< Security Manager Out Of Band Flags. */
#define ADV_TYPE_SLAVE_CONNECTION_INTERVAL_RANGE     0x12 /**< Slave Connection Interval Range. */
#define ADV_TYPE_SOLICITED_SERVICE_UUIDS_16BIT       0x14 /**< List of 16-bit Service Solicitation UUIDs. */
#define ADV_TYPE_SOLICITED_SERVICE_UUIDS_128BIT      0x15 /**< List of 128-bit Service Solicitation UUIDs. */
#define ADV_TYPE_SERVICE_DATA                        0x16 /**< Service Data - 16-bit UUID. */
#define ADV_TYPE_PUBLIC_TARGET_ADDRESS               0x17 /**< Public Target Address. */
#define ADV_TYPE_RANDOM_TARGET_ADDRESS               0x18 /**< Random Target Address. */
#define ADV_TYPE_APPEARANCE                          0x19 /**< Appearance. */
#define ADV_TYPE_ADVERTISING_INTERVAL                0x1A /**< Advertising Interval. */
#define ADV_TYPE_LE_BLUETOOTH_DEVICE_ADDRESS         0x1B /**< LE Bluetooth Device Address. */
#define ADV_TYPE_LE_ROLE                             0x1C /**< LE Role. */
#define ADV_TYPE_SIMPLE_PAIRING_HASH_C256            0x1D /**< Simple Pairing Hash C-256. */
#define ADV_TYPE_SIMPLE_PAIRING_RANDOMIZER_R256      0x1E /**< Simple Pairing Randomizer R-256. */
#define ADV_TYPE_SERVICE_DATA_32BIT_UUID             0x20 /**< Service Data - 32-bit UUID. */
#define ADV_TYPE_SERVICE_DATA_128BIT_UUID            0x21 /**< Service Data - 128-bit UUID. */
#define ADV_TYPE_URI                                 0x24 /**< URI */
#define ADV_TYPE_3D_INFORMATION_DATA                 0x3D /**< 3D Information Data. */
#define ADV_TYPE_MANUFACTURER_SPECIFIC_DATA          0xFF /**< Manufacturer Specific Data. */

/*---------------------------------------------------------------------------*/
typedef struct {
	uint16_t conn_id;
	uint8_t  peer_address[BLE_DEVICE_ADDR_LEN];
	uint8_t  peer_type;
	uint8_t  role;
	uint8_t  used;
	uint8_t  seq;
	// TODO: connection params
} nest_context_t;

extern nest_context_t m_nest_context[NEST_TOTAL_CHANNEL];
/*---------------------------------------------------------------------------*/
#include "nest-packer.h"
#include "nest-broadcaster.h"
#include "nest-scanner.h"
#include "nest-central.h"
#include "nest-serial.h"
#include "nest-opcode-tbl.h"

#include "nest-command.h"
#include "nest-command-auth.h"
#include "nest-command-lunchr.h"
#include "nest-command-bftp.h"
#include "nest-command-pipe.h"
#include "nest-command-core.h"

enum {
	NEST_BLE_ROLE_UNKNOWN = 0x00,
	NEST_BLE_ROLE_CENTRAL_CHANNEL = 0x01,
	NEST_BLE_ROLE_CENTRAL = 0x02,
	NEST_BLE_ROLE_PERIPHERAL = 0x03,
};

enum {
	NEST_BLE_UUID_TYPE_UNKNOWN = 0x00,
	NEST_BLE_UUID_TYPE_BLE = 0x01,
	NEST_BLE_UUID_TYPE_VENDOR = 0x02,
};

enum {
	NEST_BLE_HVX_TYPE_UNKNOWN = 0x00,
	NEST_BLE_HVX_TYPE_NOTIFICATION = 0x01,
	NEST_BLE_HVX_TYPE_INDICATION = 0x02,
};

enum {
	NEST_CHANNEL_TYPE_UNKNOWN = 0x00,
	NEST_CHANNEL_TYPE_SERVICE = 0x01,
	NEST_CHANNEL_TYPE_USER_DEFINED = 0x02,
};

enum {
	NEST_CENTRAL_ADV_TYPE_UNKNOWN = 0x00,
	NEST_CENTRAL_ADV_TYPE_NFD,
	NEST_CENTRAL_ADV_TYPE_DFD,
};

enum {
	NEST_CHANNEL_STAGE_IDLE = 0x00,
	NEST_CHANNEL_STAGE_INITIATING_CONN = 0x01,
	NEST_CHANNEL_STAGE_CONN_INITIATED = 0x02,
	NEST_CHANNEL_STAGE_DISCOVERING = 0x03,
	NEST_CHANNEL_STAGE_DISCOVERED = 0x04,
};

typedef struct {
	uint16_t uuid;
	uint8_t  type;
} nest_bleuuid_t;

typedef struct
{
  uint8_t uuid128[16];
} nest_bleuuid128_t;

typedef struct {
	nest_bleuuid_t 			bleuuid;
	nest_bleuuid128_t 	vendor_uuid;
	uint16_t 						handle;
} nest_bleservice_t;

typedef struct
{
  uint8_t broadcast       :1;
  uint8_t read            :1;
  uint8_t write_wo_resp   :1;
  uint8_t write           :1;
  uint8_t notify          :1;
  uint8_t indicate        :1;
  uint8_t auth_signed_wr  :1;
  uint8_t char_ext_props  :1;
} nest_blecharacteristic_props_t;

typedef struct
{
  uint8_t read            :1;
  uint8_t write           :1;
} nest_blecharacteristic_permission_t;

typedef struct {
	uint16_t															uuid;
	uint16_t          										value_handle;
	uint16_t          										cccd_handle;
	nest_blecharacteristic_props_t  			props;
	nest_blecharacteristic_permission_t   permission;
	uint8_t 															*init_value;
	uint8_t 											        init_value_len;
} nest_blecharacteristic_t;

/**@brief Event structure for @ref BLE_GATTS_EVT_WRITE. */
typedef struct
{
	nest_bleuuid_t              uuid;               /**< Attribute UUID. */
  uint16_t                    handle;             /**< Attribute Handle. */
  uint8_t                     op;                 /**< Type of write operation, see @ref BLE_GATTS_OPS. */
  uint8_t                     auth_required;      /**< Writing operation deferred due to authorization requirement. Application may use @ref sd_ble_gatts_value_set to finalise the writing operation. */
  uint16_t                    offset;             /**< Offset for the write operation. */
  uint16_t                    len;                /**< Length of the received data. */
  uint8_t                     data[1];            /**< Received data. @note This is a variable length array. The size of 1 indicated is only a placeholder for compilation.
                                                       See @ref sd_ble_evt_get for more information on how to use event structures with variable length array members. */
} nest_blegatts_write_t;

/**@brief Event structure for @ref BLE_GATTS_EVT_WRITE. */
typedef struct
{
	uint16_t min_conn_interval;         /**< Minimum Connection Interval in 1.25 ms units, see @ref BLE_GAP_CP_LIMITS.*/
	uint16_t max_conn_interval;         /**< Maximum Connection Interval in 1.25 ms units, see @ref BLE_GAP_CP_LIMITS.*/
	uint16_t slave_latency;             /**< Slave Latency in number of connection events, see @ref BLE_GAP_CP_LIMITS.*/
	uint16_t conn_sup_timeout;          /**< Connection Supervision Timeout in 10 ms units, see @ref BLE_GAP_CP_LIMITS.*/
} nest_connection_params_t;

typedef struct
{
  uint16_t          gatt_status;        /**< GATT status code for the operation, see @ref BLE_GATT_STATUS_CODES. */
  uint8_t           update : 1;         /**< If set, data supplied in p_data will be used to update the attribute value.
                                             Please note that for @ref BLE_GATTS_AUTHORIZE_TYPE_WRITE operations this bit must always be set,
                                             as the data to be written needs to be stored and later provided by the application. */
  uint16_t          offset;             /**< Offset of the attribute value being updated. */
  uint16_t          len;                /**< Length in bytes of the value in p_data pointer, see @ref BLE_GATTS_ATTR_LENS_MAX. */
  const uint8_t    *p_data;             /**< Pointer to new value used to update the attribute value. */
} nest_gatts_authorize_params_t;

/**@brief Event structure for @ref BLE_GATTS_EVT_WRITE. */
typedef struct {
	uint8_t type;
	uint8_t address[6];
} nest_device_t;

/**@brief The callback function structure of service discovery */
typedef struct {
	void (* completed)(uint16_t conn_id, uint8_t characteristic_count);
	void (* abort)(uint16_t conn_id);
} nest_service_discovery_callback_t;

typedef struct {
	uint16_t conn_id;
	nest_device_t device;
	void (* service_discovery)(const nest_bleuuid_t *uuid, nest_service_discovery_callback_t *callback);
	void (* disconnect)(void);
} nest_ble_gatt_t;

// This function initializes the Nest stack and should be called from the main()
// function of the system.
#define RANDOM_SCAN_SLEEP_TIMING			  4

typedef struct {
	uint16_t scan_sleep_time[RANDOM_SCAN_SLEEP_TIMING];
} nest_init_config_t;

void nest_stack_init(nest_init_config_t *config);
uint16_t nest_get_scan_sleep_time(void);
bool nest_is_peripheral_connected(void);

process_event_t nest_event_central_connected;
process_event_t nest_event_central_disconnect;
process_event_t nest_event_central_services_discovered;
process_event_t nest_event_peripheral_connected;
process_event_t nest_event_peripheral_disconnect;
process_event_t nest_event_buffer_free;
process_event_t nest_event_central_initiate_conn;

PROCESS_NAME(nest_ble_scan_api_process);
PROCESS_NAME(nest_ble_adv_api_process);
PROCESS_NAME(nest_governor_process);

/*---------------------------------------------------------------------------*/
// Deprecated platform-specific routines. (Interrupt context)
typedef void (*nest_gattc_get_characteristics_handler)(nest_blecharacteristic_t *characteristics);

void nest_gattc_handle_value_event(uint16_t conn_id, uint16_t handle,
																					const uint8_t *data, uint16_t length);

// void nest_tx_completed(void);

void nest_gatts_write_event(uint16_t conn_id, uint16_t handle,
																					uint8_t *data, uint16_t length);

void nest_connected_event(uint16_t conn_id, uint8_t role, const uint8_t *addr,
															uint8_t addr_type);

void nest_disconnect_event(uint16_t conn_id, uint8_t reason);

// void nest_service_discovered_event(uint16_t conn_id, bool service_found, uint8_t char_count);

void nest_scanner_result_event(nest_scanner_peer_t *report);

// void nest_controller_exit(void);
/**
 * The structure of a Nest stack in system.
 */
struct nest_driver {
	char *name;

	/** Initialize the Nest stack */
	int (* init)(void);

	/** Reset the Nest BLE resourece*/
	int (* reset)(void);

	/** Start/Stop to broadcasting */
	int (* gap_broadcasting)(nest_broadcast_params_t *params);

	/** Setup the adv data*/
	int (* gap_set_advdata)(uint8_t *adv_data, uint16_t advsize, uint8_t *advsrp_data, uint16_t adv_scan_rsp_size);

	/** Start to scan */
	int (* gap_scan)(nest_scanner_params_t *params);

	/** Connect to the peer with address and type */
	int (* gap_connect)(nest_device_t *device,  void (* timeout_callback)(void));

	/** Disconnect to the peer with connection identity */
	int (* gap_disconnect)(uint8_t conn_id);

	/** Get the local MAC address */
	int (* gap_local_addr)(uint8_t *addr);

	/** Set the gap device name */
	int (* gap_set_device_name)(const char *name, int len);

	/** Set the gap connection param */
	int (* gap_set_ppcp)(const nest_connection_params_t *params);

	/** Get the gap connection param */
	int (* gap_get_ppcp)(nest_connection_params_t *params);

	/** Update the gap connection param */
	int (* gap_update_ppcp)(uint16_t conn_id, const nest_connection_params_t *params);

	/** set the tx power */
	int (* gap_set_txpower)(int tx_power);

	/** Service discovery */
	int (* gatt_start_service_discovery)(uint8_t conn_id, nest_bleuuid_t *uuid, nest_service_discovery_callback_t *callback);

	/** Add GATT server service */
	int (* gatts_add_service)(nest_bleservice_t *service);

	/** Add GATT server characteristics */
	int (* gatts_add_characteristic)(nest_bleservice_t *p_service,
																	 nest_blecharacteristic_t *p_characteristics);

	/**  GATT server handle value notification or indication */
	int (* gatts_handle_value)(uint16_t conn_id, uint8_t type,
														 uint16_t handle, uint8_t *data, uint16_t length);

	/** Add GATT client service */
	int (* gattc_add_service)(nest_bleservice_t *service);

	/** Get GATT client service, if the requested UUID is supported by the remote device. */
  int (* gattc_get_service)(uint16_t conn_id, nest_bleuuid_t *p_srv_uuid,
	                  uint8_t char_idx, nest_blecharacteristic_t *p_characteristic);

	/** Perform a Write (Characteristic Value or Descriptor, with or without response, signed or not, long or reliable) procedure. */
	int (* gattc_write)(uint16_t conn_id, uint16_t handle, uint16_t offset,
	                    uint16_t length, uint8_t const *p_value, void (* callback)(void));
};

extern const struct nest_driver NEST;

/* TODO: driver
	SD_BLE_GATTS_SERVICE_ADD  (O)
	SD_BLE_GATTS_INCLUDE_ADD  (X)
	SD_BLE_GATTS_CHARACTERISTIC_ADD (O)
	SD_BLE_GATTS_DESCRIPTOR_ADD (X)
	SD_BLE_GATTS_VALUE_SET, (X)
	SD_BLE_GATTS_VALUE_GET, (X)

	SD_BLE_GATTS_HVX, (O)
	SD_BLE_GATTC_WRITE (O)

	BLE_GATTS_EVT_WRITE (O)

	BLE_GATTC_EVT_HVX (O)
	BLE_GATTC_EVT_WRITE_RSP
	BLE_GATTC_EVT_READ_RSP,
	BLE_GATTC_EVT_CHAR_VALS_READ_RSP,
*/

#ifdef __cplusplus
}
#endif
#endif // _NEST_H__
