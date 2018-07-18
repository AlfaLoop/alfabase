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
#include "nest-command-core.h"
#include "sys/devid.h"
#include "libs/util/xor.h"
#include "sys/pm.h"
#include "sys/systime.h"
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
static nest_command_data_t m_output;
/*---------------------------------------------------------------------------*/
static void
clear_output_data(void)
{
	memset(m_output.data, 0, 16);
	m_output.len = 0;
}
/*---------------------------------------------------------------------------*/
PROCESS(nest_core_switch_bootloader_process, "nest-switch-blmode");
NEST_COMMAND(core_switch_blmode_command,
	      nest_air_opcode_core_switch_bootloader,
	      &nest_core_switch_bootloader_process);
PROCESS(nest_core_battery_query_process, "nest-battery-query");
NEST_COMMAND(core_battery_query_command,
	      nest_air_opcode_core_battery_query,
	      &nest_core_battery_query_process);
PROCESS(nest_core_disconnect_process, "nest-core-disconnect");
NEST_COMMAND(core_channel_disconnect_command,
	      nest_air_opcode_core_channel_disconnect,
	      &nest_core_disconnect_process);
PROCESS(nest_core_time_sync_process, "nest-core-timesync");
NEST_COMMAND(core_time_sync_command,
	      nest_air_opcode_core_time_sync,
	      &nest_core_time_sync_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_core_switch_bootloader_process, ev, data)
{
	nest_command_data_t *input;
	static uint32_t key1;
	static uint8_t xor_value;
	PROCESS_BEGIN();

	input = data;
#if DEBUG_MODULE > 1
	PRINTF("[nest command core] switchblmode_process\n");
	for (uint8_t i = 0; i < input->len; i++) {
		PRINTF("%2x ", input->data[i]);
	}
	PRINTF("\n");
#endif

	clear_output_data();

	m_output.opcode = input->opcode;
	m_output.response_process = input->response_process;

	// disconnect with peer
	// NEST.gap_disconnect(input->conn_id);

	devid_bootloader_mode();

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_core_battery_query_process, ev, data)
{
	static nest_command_data_t *input;
	PROCESS_BEGIN();

	input = data;
#if DEBUG_MODULE > 1
	PRINTF("[nest command core] battery query\n");
	for (uint8_t i = 0; i < input->len; i++) {
		PRINTF("%2x ", input->data[i]);
	}
	PRINTF("\n");
#endif

	clear_output_data();

	m_output.opcode = input->opcode;
	m_output.response_process = input->response_process;

	m_output.data[0] = ENONE;
	m_output.data[1] = pm_current_mode();
	m_output.data[2] = pm_get_battery();
	m_output.len = 3;

	nest_command_send(&m_output);
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_core_disconnect_process, ev, data)
{
	nest_command_data_t *input;
	static uint32_t key1;
	static uint8_t xor_value;
	PROCESS_BEGIN();

	input = data;
#if DEBUG_MODULE > 1
	PRINTF("[nest command core] channel disconnect\n");
	for (uint8_t i = 0; i < input->len; i++) {
		PRINTF("%2x ", input->data[i]);
	}
	PRINTF("\n");
#endif

	clear_output_data();

	m_output.opcode = input->opcode;
	m_output.response_process = input->response_process;

	PRINTF("[nest command core] disconnect\n");

	// disconnect with peer
	// NEST.gap_disconnect(input->conn_id);

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_core_time_sync_process, ev, data)
{
	nest_command_data_t *input;
	static uint32_t timestamp;
	PROCESS_BEGIN();

	input = data;
#if DEBUG_MODULE > 1
	PRINTF("[nest command core] time sync\n");
	for (uint8_t i = 0; i < input->len; i++) {
		PRINTF("%2x ", input->data[i]);
	}
	PRINTF("\n");
#endif

	clear_output_data();

	m_output.opcode = input->opcode;
	m_output.response_process = input->response_process;

	// copy the timestamp (4 bytes)
	timestamp = ((uint32_t)input->data[0] << 24) | ((uint32_t)input->data[1] << 16) |
				((uint32_t)input->data[2] << 8) | ((uint32_t)input->data[3]);

	m_output.data[0] = systime_set_current_time(timestamp);
	m_output.len = 1;

	nest_command_send(&m_output);
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
nest_command_core_init(void)
{
	nest_command_register(&core_switch_blmode_command);
	nest_command_register(&core_battery_query_command);
	nest_command_register(&core_channel_disconnect_command);
	nest_command_register(&core_time_sync_command);
}
/*---------------------------------------------------------------------------*/
