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
#include "contiki.h"
#include "nest-command-lunchr.h"
#include "loader/lunchr.h"

#if defined(USE_WDUI_STACK)
#include "wdui/wdui.h"
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
#define CLEAR_OUTPUT_DATA() do { \
														  memset(m_output.data, 0, 16); \
													   	m_output.len = 0; \
													 }while(0);

#if NEST_COMMAND_LUNCHR_ENABLE_CONF == 1

// temporary variable for store the next ota app name
static uint32_t m_uuid;
static nest_command_data_t m_output;
/*---------------------------------------------------------------------------*/
PROCESS(nest_lunchr_run_with_uuid_process, "nest-lunchr-exec");
NEST_COMMAND(lunchr_run_with_uuid_command,
	      nest_air_opcode_lunchr_exec,
	      &nest_lunchr_run_with_uuid_process);
PROCESS(nest_lunchr_kill_process, "nest-lunchr-kill");
NEST_COMMAND(lunchr_kill_command,
	      nest_air_opcode_lunchr_kill,
	      &nest_lunchr_kill_process);
PROCESS(nest_lunchr_set_boot_process, "nest-lunchr-set_boot");
NEST_COMMAND(lunchr_set_boot_process_command,
	      nest_air_opcode_lunchr_set_boot_process,
	      &nest_lunchr_set_boot_process);
PROCESS(nest_lunchr_remove_boot_process, "nest-lunchr-remove_boot");
NEST_COMMAND(lunchr_remove_boot_process_command,
	      nest_air_opcode_lunchr_remove_boot_process,
	      &nest_lunchr_remove_boot_process);
PROCESS(nest_lunchr_running_query_process, "nest-lunchr-running_query");
NEST_COMMAND(lunchr_running_process_query_command,
	      nest_air_opcode_lunchr_running_query,
	      &nest_lunchr_running_query_process);
PROCESS(nest_lunchr_boot_query_process, "nest-lunchr-boot_query");
NEST_COMMAND(lunchr_boot_process_query_command,
	      nest_air_opcode_lunchr_boot_query,
	      &nest_lunchr_boot_query_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_lunchr_run_with_uuid_process, ev, data)
{
	nest_command_data_t *input;
	static uint32_t err_code;
	PROCESS_BEGIN();

	input = data;
#if DEBUG_MODULE > 1
	PRINTF("[nest lunchr command] exec process\n");
	for (uint8_t i = 0; i < input->len; i++) {
		PRINTF("%2x ", input->data[i]);
	}
	PRINTF("\n");
#endif

	CLEAR_OUTPUT_DATA();

	m_output.opcode = input->opcode;
	m_output.response_process = input->response_process;

	// uuid (4 bytes) + reserve (12 bytes) NOTE:
	if (input->len == 4) {
		m_uuid = ((uint32_t)input->data[0] << 24) | ((uint32_t)input->data[1] << 16) |
					((uint32_t)input->data[2] << 8) | ((uint32_t)input->data[3]);

#if defined(USE_WDUI_STACK)
		// wdui_nest_event_notify(NEST_LUNCHR_LOAD, &m_uuid);
		// scr_launcher_freeze();
		err_code = wdui_switch_screen(SCR_EXTENSION, m_uuid);
#else
		err_code = lunchr_load_app_with_uuid(m_uuid);
#endif

		m_output.data[0] = err_code;
		m_output.len = 1;

		nest_command_send(&m_output);
	}
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_lunchr_kill_process, ev, data)
{
	nest_command_data_t *input;
	static uint32_t err_code;
	PROCESS_BEGIN();

	input = data;

#if DEBUG_MODULE > 1
	PRINTF("[nest lunchr command] kill process\n");
#endif

	CLEAR_OUTPUT_DATA();

	m_output.opcode = input->opcode;
	m_output.response_process = input->response_process;

#if defined(USE_ELFLOADER)
	err_code = lunchr_kill_running_app();
	m_output.data[0] = err_code;
	m_output.len = 1;
#endif

#if defined(USE_WDUI_STACK)
		wdui_nest_event_notify(NEST_LUNCHR_KILL, NULL);
#endif

	nest_command_send(&m_output);
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_lunchr_set_boot_process, ev, data)
{
	nest_command_data_t *input;
	static uint32_t err_code;
	static uint32_t uuid;

	PROCESS_BEGIN();

	input = data;
#if DEBUG_MODULE > 1
	PRINTF("[nest lunchr command] set boot process\n");
#endif

	CLEAR_OUTPUT_DATA();

	m_output.opcode = input->opcode;
	m_output.response_process = input->response_process;

#if defined(USE_ELFLOADER)
	err_code = lunchr_set_boot_task(&uuid);
	if (err_code == ENONE) {
		m_output.data[1] = (uint8_t)((uuid & 0xFF000000) >> 24);
		m_output.data[2] = (uint8_t)((uuid & 0x00FF0000) >> 16);
		m_output.data[3] = (uint8_t)((uuid & 0x0000FF00) >> 8);
		m_output.data[4] = (uint8_t)((uuid & 0x000000FF));
	}
#endif
	m_output.data[0] = err_code;
	m_output.len = 5;

	nest_command_send(&m_output);
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_lunchr_remove_boot_process, ev, data)
{
	nest_command_data_t *input;
	static uint32_t err_code;

	PROCESS_BEGIN();

	input = data;
	PRINTF("[nest lunchr command] remove boot process\n");

	CLEAR_OUTPUT_DATA();

	m_output.opcode = input->opcode;
	m_output.response_process = input->response_process;

#if defined(USE_ELFLOADER)
	err_code = lunchr_remove_boot_task();
	m_output.data[0] = err_code;
	m_output.len = 1;
#endif

	nest_command_send(&m_output);
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_lunchr_running_query_process, ev, data)
{
	nest_command_data_t *input;
	static uint32_t err_code;
	static uint32_t uuid;

	PROCESS_BEGIN();

	input = data;
	PRINTF("[nest command lunchr] running query process\n");

	CLEAR_OUTPUT_DATA();

	m_output.opcode = input->opcode;
	m_output.response_process = input->response_process;

	m_output.data[0] = ENONE;
#if defined(USE_ELFLOADER)
	if (lunchr_is_running()) {
		err_code = lunchr_get_running_task_uuid(&uuid);
		if (err_code == ENONE) {
			m_output.data[1] = (uint8_t)((uuid & 0xFF000000) >> 24);
			m_output.data[2] = (uint8_t)((uuid & 0x00FF0000) >> 16);
			m_output.data[3] = (uint8_t)((uuid & 0x0000FF00) >> 8);
			m_output.data[4] = (uint8_t)((uuid & 0x000000FF));
		}
		m_output.data[0] = ENONE;
	} else {
		m_output.data[0] = EINVALSTATE;
	}
	m_output.len = 5;
#endif

	nest_command_send(&m_output);
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_lunchr_boot_query_process, ev, data)
{
	nest_command_data_t *input;
	static uint32_t err_code;
	static uint32_t uuid;
	PROCESS_BEGIN();

	input = data;
	PRINTF("[nest command lunchr] boot query process\n");

	CLEAR_OUTPUT_DATA();

	m_output.opcode = input->opcode;
	m_output.response_process = input->response_process;

#if defined(USE_ELFLOADER)
	err_code = lunchr_get_boot_task_uuid(&uuid);
	if (err_code == ENONE) {
		PRINTF("[nest command lunchr] boot uuid %8x\n", uuid);
		m_output.data[1] = (uint8_t)((uuid & 0xFF000000) >> 24);
		m_output.data[2] = (uint8_t)((uuid & 0x00FF0000) >> 16);
		m_output.data[3] = (uint8_t)((uuid & 0x0000FF00) >> 8);
		m_output.data[4] = (uint8_t)((uuid & 0x000000FF));
	} else if (err_code == ENOENT) {
		PRINTF("[nest command lunchr] no boot file\n");
	} else if (err_code == EINTERNAL) {
		PRINTF("[nest command lunchr] load boot file failed\n");
	}
	m_output.data[0] = err_code;
	m_output.len = 5;
#endif
	nest_command_send(&m_output);
	PROCESS_END();
}
#endif
/*---------------------------------------------------------------------------*/
void
nest_command_lunchr_init(void)
{
#if NEST_COMMAND_LUNCHR_ENABLE_CONF == 1
	nest_command_register(&lunchr_run_with_uuid_command);
	nest_command_register(&lunchr_kill_command);
	nest_command_register(&lunchr_set_boot_process_command);
	nest_command_register(&lunchr_remove_boot_process_command);
	nest_command_register(&lunchr_running_process_query_command);
	nest_command_register(&lunchr_boot_process_query_command);
#endif
}
/*---------------------------------------------------------------------------*/
