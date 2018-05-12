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
#include "nest.h"
#include "libs/util/byteorder.h"
#include "sys/pm.h"
#include "spiffs/spiffs.h"
#include <string.h>

#if defined(USE_FREERTOS)
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#endif

#if defined(USE_FRAMEWORK)
#include "frameworks/ble/ble_api.h"
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
#ifdef USE_FREERTOS
extern TaskHandle_t g_contiki_thread;
#endif

nest_context_t  m_nest_context[NEST_TOTAL_CHANNEL];
static struct process  *nest_scan_front_process = NULL;
static struct process  *nest_adv_front_process = NULL;
// Array of structures containing information about the registered gattc services
static nest_bleuuid_t m_gattc_services_registered[NEST_BLE_DISCOVERY_MAX_SRV];
static uint8_t m_num_of_gattc_services_reg;
static uint8_t 				 m_nest_initiate_peer_addr[BLE_DEVICE_ADDR_LEN];
static uint8_t 				 m_nest_initiate_peer_addr_type;
static bool 					 m_nest_initiate_central_flag = false;
static bool            m_nest_pheripheral_connected = false;
static nest_init_config_t m_nest_setting;
static struct nest_serial_driver m_serial_driver;
static struct timer debounce_scan_timer;
/*---------------------------------------------------------------------------*/
static void
central_store_peer_info(uint8_t *peer_addr, uint8_t peer_type)
{
	m_nest_initiate_central_flag = true;
	memcpy(m_nest_initiate_peer_addr, peer_addr, BLE_DEVICE_ADDR_LEN);
	m_nest_initiate_peer_addr_type = peer_type;
}
/*---------------------------------------------------------------------------*/
#if defined(USE_FRAMEWORK)
/*---------------------------------------------------------------------------*/
PROCESS(nest_ble_scan_api_process, "nest_ble_api_scan_process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_ble_scan_api_process, ev, data)
{
	PROCESS_BEGIN();
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL( (ev == PROCESS_EVENT_CONTINUE) );

		if (process_is_running(&nest_scan_central_process))
			PROCESS_WAIT_EVENT_UNTIL( (ev == PROCESS_EVENT_EXITED && data == &nest_scan_central_process));

		if (process_is_running(&nest_broadcast_api_process))
			PROCESS_WAIT_EVENT_UNTIL( (ev == PROCESS_EVENT_EXITED && data == &nest_broadcast_api_process));

		if (!process_is_running(&nest_scan_api_process)) {
			process_start(&nest_scan_api_process, NULL);
			// Block for the timeout
			if (process_is_running(&nest_scan_api_process)) {
				nest_scan_front_process = &nest_ble_scan_api_process;
				PROCESS_WAIT_EVENT_UNTIL( (ev == PROCESS_EVENT_EXITED && data == &nest_scan_api_process) ||
																	(ev == nest_event_central_initiate_conn));
				if (ev == nest_event_central_initiate_conn) {
					process_post(&nest_scan_api_process, PROCESS_EVENT_EXIT, NULL);
					PROCESS_WAIT_EVENT_UNTIL( (ev == PROCESS_EVENT_EXITED && data == &nest_scan_api_process));
					ble_scan_completed(NEST_SCANNER_ABORT);
					m_nest_initiate_central_flag = false;
					nest_central_initiate_connection(m_nest_initiate_peer_addr, m_nest_initiate_peer_addr_type);
				} if ((ev == PROCESS_EVENT_EXITED && data == &nest_scan_api_process)) {
					ble_scan_completed(NEST_SCANNER_TIMEOUT);
				}
			}
			timer_set(&debounce_scan_timer, 6000);
			nest_scan_front_process = NULL;
		}
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS(nest_ble_adv_api_process, "nest_ble_adv_api_process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_ble_adv_api_process, ev, data)
{
	static int err_code;
	static nest_broadcast_params_t adv_params;
	static struct etimer adv_etimer;
	static uint16_t conn_handle;
	static nest_connection_params_t conn_params;

	PROCESS_BEGIN();
	nest_adv_front_process = &nest_ble_adv_api_process;

	while(1) {
		PROCESS_WAIT_EVENT_UNTIL( (ev == PROCESS_EVENT_CONTINUE )  );

		do {
			ble_adv_api_params_retrieve(&adv_params);

			if (process_is_running(&nest_scan_central_process))
				PROCESS_WAIT_EVENT_UNTIL( (ev == PROCESS_EVENT_EXITED && data == &nest_scan_central_process));

			if (process_is_running(&nest_scan_api_process))
				PROCESS_WAIT_EVENT_UNTIL( (ev == PROCESS_EVENT_EXITED && data == &nest_scan_api_process));

			if (!process_is_running(&nest_broadcast_api_process)) {

				// check the tx power
				if (ble_api_get_tx_power() != PLATFORM_DEVICE_TX_POWER /*&&
					!process_is_running(&nest_central_process) */)
				{
					NEST.gap_set_txpower(ble_api_get_tx_power());
				}

				etimer_set(&adv_etimer, adv_params.interval);
				process_start(&nest_broadcast_api_process, NULL);
				if (process_is_running(&nest_broadcast_api_process)) {
					nest_adv_front_process = &nest_broadcast_api_process;
					// PRINTF("[ble adv api process] start broadcast\n");
					PROCESS_WAIT_EVENT_UNTIL( (ev == PROCESS_EVENT_EXITED && data == &nest_broadcast_api_process) ||
																		(ev == nest_event_peripheral_connected));

					if (ev == nest_event_peripheral_connected) {
						//process_exit(&nest_broadcast_api_process);
						// set the tx power to default range
						NEST.gap_set_txpower(PLATFORM_DEVICE_TX_POWER);

						// cancel evnet timer
 						etimer_stop(&adv_etimer);

						/*PRINTF("[ble adv api process] connected event\n");
						err_code = NEST.gap_get_ppcp(&conn_params);
						if (err_code == ENONE) {
							PRINTF("[nest] min internval %d max internval %d latency %d timeout %d\n",
								conn_params.min_conn_interval, conn_params.max_conn_interval,
								conn_params.slave_latency, conn_params.conn_sup_timeout);
						}*/
						 // issue the connection event
						conn_handle = data;
						ble_api_connection_event_handler(conn_handle, BLE_GATT_STATE_CONNECTED);
						m_nest_pheripheral_connected = true;

						// wait for the disconnect event
						PROCESS_WAIT_EVENT_UNTIL( (ev == nest_event_peripheral_disconnect) );
						PRINTF("[ble adv api process] nest_disconnect_event event\n");

						conn_handle = data;
						ble_api_connection_event_handler(conn_handle, BLE_GATT_STATE_DISCONNECTED);
						m_nest_pheripheral_connected = false;
					 } else {
						// issue the send out event to api layer
						// set the tx power to default
						NEST.gap_set_txpower(PLATFORM_DEVICE_TX_POWER);

						// PRINTF("[nest broadcasting] sendout\n");
						ble_adv_api_sendout();
						// PRINTF("[nest broadcasting] wait for the next interval\n");
						PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&adv_etimer));
						etimer_stop(&adv_etimer);
					}
				}
				nest_adv_front_process = &nest_ble_adv_api_process;
			}
		} while (ble_adv_api_attached());
	}
	PROCESS_END();
}
#endif /* USE_FRAMEWORK */
/*---------------------------------------------------------------------------*/
PROCESS(nest_governor_process, "nest governor");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_governor_process, ev, data)
{
	static nest_scanner_params_t params;
	static uint32_t ret;
	static uint8_t error_count = 0;
	static struct etimer timeout_etimer;
	static struct etimer sleep_etimer;
	static uint32_t app_sleep_time;

	PROCESS_BEGIN();
	while(1) {
		// sleep for power saving
		app_sleep_time = nest_get_scan_sleep_time();
		etimer_set(&sleep_etimer, app_sleep_time);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sleep_etimer));
#ifdef USE_FRAMEWORK
		// scan conditions
		if (!process_is_running(&nest_scan_api_process) &&
				!ble_scan_api_attached() &&
			  nest_central_status() == NEST_CENTRAL_STATUS_NONE &&
				timer_expired(&debounce_scan_timer) )
#else
		if (timer_expired(&debounce_scan_timer) )
#endif
		{
			PRINTF("[nest governor] sleep %d\n", app_sleep_time);
			if ( (PM.get_charging_status() == PM_SOURCE_CHARGING) ||
           (pm_current_mode() == PM_NORMAL_MODE) )
			{
#if defined(USE_FRAMEWORK)
				if (process_is_running(&nest_broadcast_api_process))
					PROCESS_WAIT_EVENT_UNTIL( (ev == PROCESS_EVENT_EXITED && data == &nest_broadcast_api_process));
#endif
				NEST.gap_set_txpower(PLATFORM_DEVICE_TX_POWER);
				process_start(&nest_scan_central_process, NULL);
				if (process_is_running(&nest_scan_central_process)) {
					nest_scan_front_process = &nest_governor_process;
					PROCESS_WAIT_EVENT_UNTIL( (ev == PROCESS_EVENT_EXITED && data == &nest_scan_central_process) ||
																		(ev == nest_event_central_initiate_conn));
					if (ev == nest_event_central_initiate_conn) {
						PRINTF("[nest governor] post exit evnet to scanner\n");
						// process_exit(&nest_scan_central_process);
						process_post(&nest_scan_central_process, PROCESS_EVENT_EXIT, NULL);
						PRINTF("[nest governor] wait for the exited evnet from scanner\n");
						PROCESS_WAIT_EVENT_UNTIL( (ev == PROCESS_EVENT_EXITED && data == &nest_scan_central_process));
						PRINTF("[nest governor] scanner is exited\n");
						m_nest_initiate_central_flag = false;
						nest_central_initiate_connection(m_nest_initiate_peer_addr, m_nest_initiate_peer_addr_type);
					}
				}
				nest_scan_front_process = NULL;
			}
			else
			{
				PRINTF("[nest governor] saving power\n");
			}
		}
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
bool
nest_is_peripheral_connected(void)
{
	return m_nest_pheripheral_connected;
}
/*---------------------------------------------------------------------------*/
void
nest_gattc_handle_value_event(uint16_t conn_id, uint16_t handle,
																	const uint8_t *data, uint16_t length)
{
	// HVX can only occur from client sending.
	PRINTF("[nest] gattc handle value length %d handle: %d\n", length, handle);
#if NEST_CHANNEL_CENTRAL_CONF == 1
	if (m_nest_context[conn_id].role == NEST_BLE_ROLE_CENTRAL_CHANNEL)
	{
		nest_central_gattc_handle_value_event(m_nest_context[conn_id].conn_id, handle, data, length);
	}
#endif /* NEST_CHANNEL_CENTRAL_CONF */

	// TODO: pass to the framework layer
#if defined(USE_FRAMEWORK)

#endif  /* USE_FRAMEWORK */
}
/*---------------------------------------------------------------------------*/
void
nest_gatts_write_event(uint16_t conn_id, uint16_t handle,
																				uint8_t *data, uint16_t length)
{
  // pass handle to api layer
#if defined(USE_FRAMEWORK)
	ble_gatts_write_event_handler(conn_id, handle, data, length);
#endif
}
/*---------------------------------------------------------------------------*/
void
nest_connected_event(uint16_t conn_id, uint8_t role, const uint8_t *addr,
																														uint8_t addr_type)
{
	// prepare to dispacher the connected event (interrupt content, no need to send semaphore)
	PRINTF("[nest] connected: %d role %d\n", conn_id, role);
	// if nest channel process is in running, compare the peer address.
	if ( role == NEST_BLE_ROLE_CENTRAL)
	{
		m_nest_context[conn_id].conn_id = conn_id;
		m_nest_context[conn_id].used = 1;
		m_nest_context[conn_id].seq = 0;
		m_nest_context[conn_id].peer_type = addr_type;
		m_nest_context[conn_id].role = NEST_BLE_ROLE_CENTRAL;
		memcpy(m_nest_context[conn_id].peer_address, addr, BLE_DEVICE_ADDR_LEN);

#if NEST_CHANNEL_CENTRAL_CONF == 1
		if ( (memcmp(addr, m_nest_initiate_peer_addr, BLE_DEVICE_ADDR_LEN) == 0) &&
				 (addr_type == m_nest_initiate_peer_addr_type) )
		{
			m_nest_context[conn_id].role = NEST_BLE_ROLE_CENTRAL_CHANNEL;
			nest_central_connected_event(conn_id, addr, addr_type);
		}
#endif
		// TODO: pass the connected event to framework layer

	} else if ( role == NEST_BLE_ROLE_PERIPHERAL) {
		// TODO: pass the connected event to framework layer
		PRINTF("[nest] peripheral connected\n");
		m_nest_context[conn_id].conn_id = conn_id;
		m_nest_context[conn_id].used = 1;
		m_nest_context[conn_id].seq = 0;
		m_nest_context[conn_id].peer_type = addr_type;
		m_nest_context[conn_id].role = NEST_BLE_ROLE_PERIPHERAL;
		memcpy(m_nest_context[conn_id].peer_address, addr, BLE_DEVICE_ADDR_LEN);
		// nest_pheripheral_connected_event(&m_nest_context[conn_id]);
		// if (nest_adv_front_process == &nest_broadcast_api_process) {
		PRINTF("[nest] connected post broadcaster event\n");
		uint16_t data = m_nest_context[conn_id].conn_id;

#if defined(USE_FRAMEWORK)
		process_post(&nest_ble_adv_api_process, nest_event_peripheral_connected, data);
#endif
	}
}
/*---------------------------------------------------------------------------*/
void
nest_disconnect_event(uint16_t conn_id, uint8_t reason)
{
	PRINTF("[nest] disconnect %d\n", conn_id);
	m_nest_context[conn_id].conn_id = conn_id;
	m_nest_context[conn_id].used = 0;
	m_nest_context[conn_id].seq = 0;

	// dispatcher the event
#if NEST_CHANNEL_CENTRAL_CONF == 1
		if (m_nest_context[conn_id].role == NEST_BLE_ROLE_CENTRAL_CHANNEL) {
			nest_central_disconnect_event(conn_id, reason);
		}
#endif
	if (m_nest_context[conn_id].role == NEST_BLE_ROLE_CENTRAL) {
		// TODO: pass the event to framework layer

	} else if ( m_nest_context[conn_id].role == NEST_BLE_ROLE_PERIPHERAL) {
		PRINTF("[nest] disconnect post broadcaster event\n");
		uint16_t data = m_nest_context[conn_id].conn_id;

#ifdef USE_FRAMEWORK
		process_post(&nest_ble_adv_api_process, nest_event_peripheral_disconnect, data);
#endif
	}
	// set to default
	m_nest_context[conn_id].role = NEST_BLE_ROLE_UNKNOWN;
}
/*---------------------------------------------------------------------------*/
void
nest_tx_completed(void)
{
	// PRINTF("[nest] tx completed event\n");
	// if ( process_is_running(&nest_central_process)) {
	//  process_post(&nest_central_process, nest_channel_event_tx_completed, NULL);
	// }
	process_post(PROCESS_BROADCAST, nest_event_buffer_free, NULL);
}
/*---------------------------------------------------------------------------*/
void
nest_scanner_result_event(nest_scanner_peer_t *report)
{
	// Interrupt context
	uint32_t err_code;
	uint8_t  nest_adv_type = NEST_CENTRAL_ADV_TYPE_UNKNOWN;

	// scan response data
	if (report->scan_response && report->len > 0) {
		// process scan response data
#if DEBUG_MODULE > 1
		PRINTF("[nest] adv scan_rsp type: %d, length:%d data: ", report->type, report->len);
		for (int i = 0; i < report->len; i++) {
			PRINTF("%02x ", report->data[i]);
		}
		PRINTF("\n");
#endif

#if defined(USE_FRAMEWORK)
		ble_scan_api_event_report(report);
#endif

	} else {

#if DEBUG_MODULE > 1
		PRINTF("[nest] adv type: %d, length:%d data: ", report->type, report->len);
		for (int i = 0; i < report->len; i++) {
			PRINTF("%02x ", report->data[i]);
		}
		PRINTF("\n");
#endif

#if NEST_CHANNEL_CENTRAL_CONF == 1
		if (!m_nest_initiate_central_flag)
		{
			nest_adv_type = nest_central_adv_filter(report->data, report->len);
			if ( (nest_adv_type == NEST_CENTRAL_ADV_TYPE_NFD && report->rssi > NEST_ADV_TYPE_NFD_RSSI) ||
					 (nest_adv_type == NEST_CENTRAL_ADV_TYPE_DFD) )
		  {
				// Store the broadcaster and start initiate the connection
				central_store_peer_info(report->peer_address, report->peer_addr_type);

				// stop the nest scan process
				if (nest_scan_front_process != NULL) {
					process_post(nest_scan_front_process, nest_event_central_initiate_conn, NULL);
				}
			}
		}
#endif

		if (nest_adv_type == NEST_CENTRAL_ADV_TYPE_UNKNOWN) {
#if defined(USE_FRAMEWORK)
			ble_scan_api_event_report(report);
#endif
		}
	}
}
/*---------------------------------------------------------------------------*/
void
nest_service_discovered_event(uint16_t conn_id, bool service_found, uint8_t char_count)
{
	//TODO: Callback invoked when the list of remote services, characteristics and
	// descriptors for the remote device have been updated, ie new services have been discovered.
}
/*---------------------------------------------------------------------------*/
uint16_t
nest_get_scan_sleep_time(void)
{
	uint16_t sleep_time;
	uint16_t random_value = random_rand();
	sleep_time = m_nest_setting.scan_sleep_time[random_value % (RANDOM_SCAN_SLEEP_TIMING)];
	//PRINTF("[nest] sleep time %d %d %d\n", sleep_time, random_value, random_value % (RANDOM_SCAN_SLEEP_TIMING));
	return sleep_time;
}
/*---------------------------------------------------------------------------*/
void
nest_stack_init(nest_init_config_t *config)
{
	uint32_t err_code;
	if ( NEST.init() != ENONE) {
	//	PRINTF("[NEST] init failed\n");
		return;
	}

	// copy the parameters
	memcpy(m_nest_setting.scan_sleep_time, config->scan_sleep_time, sizeof(uint16_t)*RANDOM_SCAN_SLEEP_TIMING);

	nest_event_peripheral_connected = process_alloc_event();
	nest_event_peripheral_disconnect = process_alloc_event();
	nest_event_central_connected = process_alloc_event();
	nest_event_central_disconnect = process_alloc_event();
	nest_event_central_services_discovered = process_alloc_event();
	nest_event_central_initiate_conn = process_alloc_event();
	nest_event_buffer_free = process_alloc_event();

	// initialize multi-role system
	nest_broadcaster_init();
	nest_scanner_init();

	// initialize ble central interface
#if NEST_CHANNEL_CENTRAL_CONF == 1
	nest_central_init();
#endif

	// initialize serial interface
#if NEST_CHANNEL_SERIAL_CONF == 1
	nest_serial_init();
#endif

	// initialize command system
#if NEST_COMMAND_ENABLE_CONF == 1
	nest_command_init();
#if NEST_COMMAND_BFTP_ENABLE_CONF == 1
	nest_command_bftp_init();
#endif /* NEST_COMMAND_BFTP_ENABLE_CONF */
#if NEST_COMMAND_CORE_ENABLE_CONF == 1
	nest_command_core_init();
#endif /* NEST_COMMAND_CORE_ENABLE_CONF */
#if NEST_COMMAND_LUNCHR_ENABLE_CONF == 1
	nest_command_lunchr_init();
#endif /* NEST_COMMAND_LUNCHR_ENABLE_CONF */
#if NEST_COMMAND_AUTH_ENABLE_CONF == 1
	nest_command_auth_init();
#endif /* NEST_COMMAND_AUTH_ENABLE_CONF */
#if NEST_COMMAND_PIPE_ENABLE_CONF == 1
	nest_command_pipe_init();
#endif   /* NEST_COMMAND_PIPE_ENABLE_CONF */
#endif  /* NEST_COMMAND_ENABLE_CONF */

#if defined(USE_FRAMEWORK)
	if (!process_is_running(&nest_ble_scan_api_process))
		process_start(&nest_ble_scan_api_process, NULL);

	if (!process_is_running(&nest_ble_adv_api_process))
		process_start(&nest_ble_adv_api_process, NULL);

	if (!process_is_running(&nest_governor_process))
		process_start(&nest_governor_process, NULL);
#endif /* USE_FRAMEWORK */
}
/*---------------------------------------------------------------------------*/
