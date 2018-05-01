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
#ifndef _NEST_DRIVER_H__
#define _NEST_DRIVER_H__
#ifdef __cplusplus
extern "C" {
#endif

#include "nest/nest.h"
#include <stdint.h>

int is_radio_accessing(void);

int nrf_gap_broadcasting(nest_broadcast_params_t *params);
int nrf_gap_scanner(nest_scanner_params_t *params);

// nest-driver-gap-connect.c
int nrf_gap_connect(nest_device_t *device, void (* timeout_callback)(void));
void nrf_gap_central_connect_timeout(void);

// nest-driver-gap-disconnect.c
int nrf_gap_disconnect(uint16_t conn_id);

int nrf_gatts_add_service(nest_bleservice_t *service);
int nrf_gatts_init_service_pool(void);

int nrf_gatts_add_characteristic(nest_bleservice_t *p_service,
                                 nest_blecharacteristic_t *p_characteristics);
int nrf_gatts_init_characteristic_pool(void);

int nrf_gatts_handle_value(uint16_t conn_id, uint8_t type,
                           uint16_t handle, uint8_t *data, uint16_t length);

// nest-driver-gattc-write.c
int nrf_gattc_write(uint16_t conn_id, uint16_t handle, uint16_t offset,
                    uint16_t length, uint8_t const *p_value, void (* callback)(void));
void nrf_gattc_write_tx_complete(void);

int nrf_getaddr(uint8_t *macaddr);

int nrf_gap_set_device_name(const char *name, int length);

int nrf_gap_set_ppcp(const nest_connection_params_t *params);

int nrf_gap_get_ppcp(nest_connection_params_t *params);

int nrf_gap_update_ppcp(uint16_t conn_id, const nest_connection_params_t *params);

int nrf_gap_set_txpower(int tx_power);

int nrf_gap_set_adv_data(uint8_t *adv_data, uint16_t advsize, uint8_t *advsrp_data,
                                                    uint16_t adv_scan_rsp_size);

PROCESS_NAME(nrf_sd_event_process);

process_event_t nrf_sd_event_gap_connected;
process_event_t nrf_sd_event_gap_disconnect;
process_event_t nrf_sd_event_gattc_hvx;
process_event_t nrf_sd_event_gatts_write;
process_event_t nrf_sd_event_gap_connect_timeout;
process_event_t nrf_sd_event_tx_completed;
process_event_t nrf_sd_event_services_discovered;
process_event_t nrf_sd_event_adv_report;

extern const struct nest_driver nrf_nest_driver;

#ifdef __cplusplus
}
#endif
#endif /* _NEST_DRIVER_H__ */
