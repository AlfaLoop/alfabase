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
#ifndef _NEST_SCANNER_H_
#define _NEST_SCANNER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// This module hanndle the actived by (is_nest_scanner_registed).
// It provide the process to handle ble scanner flow.
// is_nest_scanner_registed is actived by framework layer (nest-sysc)
enum {
	NEST_SCANNER_ABORT = 0x00,
	NEST_SCANNER_TIMEOUT,
	NEST_SCANNER_RESULT
};

typedef struct {
	uint8_t   peer_addr_type;
	uint8_t 	peer_address[BLE_DEVICE_ADDR_LEN];
	int8_t    rssi;
	uint8_t		scan_response:1;
	uint8_t 	type:2;
	uint8_t   len:5;
	uint8_t   data[ADV_MAX_DATA_SIZE];
} nest_scanner_peer_t;

typedef struct {
	uint8_t	 active;
	uint32_t interval;
	uint16_t window;
} nest_scanner_params_t;

// controller by nest stack
void nest_scanner_init(void);
bool nest_scanner_scanning(void);

process_event_t nest_event_scanner_timeout;

PROCESS_NAME(nest_scan_api_process);
PROCESS_NAME(nest_scan_central_process);

#ifdef __cplusplus
}
#endif
#endif /* _NEST_SCANNER_H_ */
