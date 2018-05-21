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
#include <string.h>
#include "nest-command-bftp.h"
#include "sys/ctimer.h"
#include "sys/filesystem.h"
#include "loader/lunchr.h"
#include "libs/util/xor.h"
#include "spiffs/spiffs.h"
#include "bsp_init.h"

#ifdef USE_ELFLOADER
#include "loader/elfloader.h"
#endif

// FreeRTOS
#ifdef USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
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
#ifdef STORAGE_SYSC_FS_INSTANCE_CONF
#define SFS STORAGE_SYSC_FS_INSTANCE_CONF
#else
#define SFS SYSFS
#endif
/*---------------------------------------------------------------------------*/

enum {
	NEST_COMMAND_BFTP_PREFIX_NAME = 0x00,
	NEST_COMMAND_BFTP_PREFIX_APP = 0x01,
	NEST_COMMAND_BFTP_PREFIX_FILE_TYPE = 0x02,
	NEST_COMMAND_BFTP_PREFIX_EXT_FILE = 0x03,
	NEST_COMMAND_BFTP_PREFIX_ICON = 0x04,
	NEST_COMMAND_BFTP_PREFIX_PEDOMETER = 0x05,
	NEST_COMMAND_BFTP_PREFIX_TOTAL
};

enum {
	BFTP_TIMEOUT_SET_INIT = 0x00,
	BFTP_TIMEOUT_SET_PACKETS,
	BFTP_TIMEOUT_STOP,
};

#define NEST_COMMAND_BFTP_SEND_TIMEOUT      3000
#define CLEAR_OUTPUT_DATA do { \
														memset(m_output.data, 0, 16); \
														m_output.len = 0; \
													} while(0);

#define BATCHSIZE 				SPIFFS_COPY_BUFFER_STACK
#define DATA_PACKETS_MAX					     16

#if NEST_COMMAND_BFTP_ENABLE_CONF == 1

// temporary variable for store the bftp
static uint8_t m_bftp_operate_type;
static uint32_t m_file_uuid = 0;
static uint32_t m_file_size = 0;
static uint32_t m_ext_file_uuid = 0;

static uint8_t m_bftp_processing = 0;

static char m_concat_buffer[FILE_CONCAT_BUFFER_SIZE];
static char m_concat_ext_buffer[EXT_FILE_CONCAT_BUFFER_SIZE];

static uint8_t m_developer_mode = 0;
static uint8_t batch[BATCHSIZE];
static uint32_t batch_num = 0;

static spiffs_file m_fd;
static uint8_t m_file_checksum = 0;
static uint32_t m_file_wr_process_size = 0;
static uint32_t m_file_last_packets = 0;
static uint32_t m_file_total_notify_num = 0;
static uint32_t m_file_wr_process_size_num = 0;
static uint8_t m_retries = 0;
static uint8_t m_total_retries = 0;

static nest_command_data_t m_output;
/*---------------------------------------------------------------------------*/
static void
clear_output_data(void)
{
	memset(m_output.data, 0, 16);
	m_output.len = 0;
}
/*---------------------------------------------------------------------------*/
PROCESS(bftp_timeout_process, "bftp_timeout_process");
/*---------------------------------------------------------------------------*/
PROCESS(nest_bftp_init_process, "nest-bftp_init");
NEST_COMMAND(bftp_init_command,
	      nest_air_opcode_bftp_init,
	      &nest_bftp_init_process);
PROCESS(nest_bftp_packets_process, "nest-bftp_packets");
NEST_COMMAND(bftp_packets_command,
	      nest_air_opcode_bftp_packets,
	      &nest_bftp_packets_process);
PROCESS(nest_bftp_end_process, "nest-bftp_end");
NEST_COMMAND(bftp_end_command,
	      nest_air_opcode_bftp_end,
	      &nest_bftp_end_process);
PROCESS(nest_bftp_remove_process, "nest-bftp_remove");
NEST_COMMAND(bftp_remove_command,
	      nest_air_opcode_bftp_remove,
	      &nest_bftp_remove_process);
PROCESS(nest_bftp_stat_process, "nest-bftp_stat");
NEST_COMMAND(bftp_stat_command,
	      nest_air_opcode_bftp_stat,
	      &nest_bftp_stat_process);
PROCESS(nest_bftp_space_used_process, "nest-bftp_space_used");
NEST_COMMAND(bftp_space_used_command,
	      nest_air_opcode_bftp_space_used,
	      &nest_bftp_space_used_process);
PROCESS(nest_bftp_readdir_process, "nest-bftp_readdir");
NEST_COMMAND(bftp_readdir_command,
	      nest_air_opcode_bftp_readdir,
	      &nest_bftp_readdir_process);
/*---------------------------------------------------------------------------*/
static void
bftp_init_timer_callback_handler(void)
{
	PRINTF("[nest command bftp] init not response: close the file\n");

	uint32_t timeout_type = BFTP_TIMEOUT_STOP;
	process_post_synch(&bftp_timeout_process, PROCESS_EVENT_CONTINUE, timeout_type);
	m_bftp_processing = false;
	m_file_wr_process_size = 0;
	m_file_checksum = 0;
	m_total_retries = 0;
	m_file_wr_process_size_num = 0;
	m_file_total_notify_num = 0;
	m_file_last_packets = 0;
	SPIFFS_close(&SYSFS, m_fd);

#if defined(USE_WDUI_STACK)
	wdui_nest_event_notify(NEST_BFTP_TIMEOUT, NULL);
#endif
}
/*---------------------------------------------------------------------------*/
static void
bftp_packtes_timer_callback_handler(void)
{
	uint32_t timeout_type;
	// resend
	m_retries++;
	m_total_retries++;
	if (m_retries >= 5) {
		// stop retry
		timeout_type = BFTP_TIMEOUT_STOP;
		process_post_synch(&bftp_timeout_process, PROCESS_EVENT_CONTINUE, timeout_type);
		m_retries = 0;
		m_bftp_processing = false;
		m_file_wr_process_size = 0;
		m_file_checksum = 0;
		m_total_retries = 0;
		m_file_wr_process_size_num = 0;
		m_file_total_notify_num = 0;
		m_file_last_packets = 0;
		m_file_wr_process_size_num = 0;
		SPIFFS_close(&SYSFS, m_fd);
		PRINTF("[nest command bftp] stop retries\n");
#if defined(USE_WDUI_STACK)
		wdui_nest_event_notify(NEST_BFTP_TIMEOUT, NULL);
#endif
	} else {
		nest_command_send(&m_output);
		timeout_type = BFTP_TIMEOUT_SET_PACKETS;
		process_post_synch(&bftp_timeout_process, PROCESS_EVENT_CONTINUE, timeout_type);
		PRINTF("[nest command bftp] miss data, resend\n");
	}
}
/*---------------------------------------------------------------------------*/
static uint8_t
is_file_type_prefer_stored_external(void)
{
	uint8_t external = 0;
	uint8_t prefix = m_bftp_operate_type & 0x7F;
	if (prefix == NEST_COMMAND_BFTP_PREFIX_EXT_FILE) {
		external = 1;
	}
	return external;
}
/*---------------------------------------------------------------------------*/
static char*
get_prefix(uint8_t filetype)
{
	char *prefix;

	if (filetype == NEST_COMMAND_BFTP_PREFIX_NAME) {
		prefix = "n/";
	} else if (filetype == NEST_COMMAND_BFTP_PREFIX_APP) {
		prefix = "a/";
	} else if (filetype == NEST_COMMAND_BFTP_PREFIX_FILE_TYPE) {
		prefix = "t/";
	} else if (filetype == NEST_COMMAND_BFTP_PREFIX_ICON) {
		prefix = "i/";
	} else if (filetype == NEST_COMMAND_BFTP_PREFIX_EXT_FILE) {
		prefix = "s/";
	} else if (filetype == NEST_COMMAND_BFTP_PREFIX_PEDOMETER) {
		prefix = "p/";
	} else {
		prefix = "z/";
	}
	return prefix;
}
/*---------------------------------------------------------------------------*/
static bool
check_and_setup_concat_buffer(void)
{
	bool is_file_support = false;
	uint8_t prefix = m_bftp_operate_type & 0x7F;

	if (prefix == NEST_COMMAND_BFTP_PREFIX_NAME) {
		sprintf(&m_concat_buffer[0], "n/%08x", m_file_uuid);
		is_file_support = true;
	} else if (prefix == NEST_COMMAND_BFTP_PREFIX_APP) {
		sprintf(&m_concat_buffer[0], "a/%08x", m_file_uuid);
		is_file_support = true;
	} else if (prefix == NEST_COMMAND_BFTP_PREFIX_FILE_TYPE) {
		sprintf(&m_concat_buffer[0], "t/%08x", m_file_uuid);
		is_file_support = true;
	} else if (prefix == NEST_COMMAND_BFTP_PREFIX_ICON) {
		sprintf(&m_concat_buffer[0], "i/%08x", m_file_uuid);
		is_file_support = true;
	} else if (prefix == NEST_COMMAND_BFTP_PREFIX_EXT_FILE) {
		sprintf(&m_concat_ext_buffer[0], "s/%08x", m_file_uuid);
		sprintf(&m_concat_ext_buffer[10], "/%08x", m_ext_file_uuid);
		is_file_support = true;
	} else if (prefix == NEST_COMMAND_BFTP_PREFIX_PEDOMETER) {
		sprintf(&m_concat_buffer[0], "p/%08x", m_file_uuid);
		is_file_support = true;
	}

	return is_file_support;
}
/*---------------------------------------------------------------------------*/
static uint32_t
bftp_init_read_file(uint8_t *buffer, uint32_t *process_len)
{
	int res;
	spiffs_stat s;
	bool is_file_support = check_and_setup_concat_buffer();
	uint8_t prefix = m_bftp_operate_type & 0x7F;

	if (!is_file_support) {
		return ENOSUPPORT;
	}

	if (prefix == NEST_COMMAND_BFTP_PREFIX_EXT_FILE) {

		res = SPIFFS_stat(&SFS, m_concat_ext_buffer, &s);
		if (res == SPIFFS_ERR_NOT_FOUND) {
			return ENOFOUND;
		}

		if (m_file_size > s.size) {
			return EINVAL;
		}

		// Open the file
		m_fd = SPIFFS_open(&SFS, m_concat_ext_buffer,  SPIFFS_RDONLY , 0);
		if(m_fd < 0) {
			SPIFFS_close(&SFS, m_fd);
			PRINTF("[nest command bftp] init: failed to rewrite file %s\n", m_concat_ext_buffer);
			return ENOENT;
		}

		if (m_file_size > 16) {
			SPIFFS_read(&SFS, m_fd, buffer, 16);
			*process_len = 16;
		} else {
			SPIFFS_read(&SFS, m_fd, buffer, m_file_size);
			*process_len = m_file_size;
		}
	} else {

		res = SPIFFS_stat(&SYSFS, m_concat_buffer, &s);
		if (res == SPIFFS_ERR_NOT_FOUND) {
			return ENOFOUND;
		}

		if (m_file_size > s.size) {
			return EINVAL;
		}

		// Open the file
		m_fd = SPIFFS_open(&SYSFS, m_concat_buffer,  SPIFFS_RDONLY , 0);
		if(m_fd < 0) {
			SPIFFS_close(&SYSFS, m_fd);
			PRINTF("[nest command bftp] init: failed to rewrite file %s\n", m_concat_buffer);
			return ENOENT;
		}

		if (m_file_size > 16) {
			SPIFFS_read(&SYSFS, m_fd, buffer, 16);
			*process_len = 16;
		} else {
			SPIFFS_read(&SYSFS, m_fd, buffer, m_file_size);
			*process_len = m_file_size;
		}
	}
	return ENONE;
}
/*---------------------------------------------------------------------------*/
static uint32_t
bftp_init_write_file(void)
{
	bool is_file_support = check_and_setup_concat_buffer();
	uint8_t prefix = m_bftp_operate_type & 0x7F;

	if (!is_file_support) {
		return ENOSUPPORT;
	}

	if (prefix == NEST_COMMAND_BFTP_PREFIX_EXT_FILE) {
		SPIFFS_remove(&SFS, m_concat_ext_buffer);
		// Open the file
		m_fd = SPIFFS_open(&SFS, m_concat_ext_buffer, SPIFFS_APPEND | SPIFFS_TRUNC | SPIFFS_CREAT  | SPIFFS_WRONLY , 0);
		if(m_fd < 0) {
			SPIFFS_close(&SFS, m_fd);
			PRINTF("[nest command bftp] init: failed to rewrite file %s\n", m_concat_ext_buffer);
			return ENOENT;
		}
	} else {
		SPIFFS_remove(&SYSFS, m_concat_buffer);
		// Open the file
		m_fd = SPIFFS_open(&SYSFS, m_concat_buffer, SPIFFS_APPEND | SPIFFS_TRUNC | SPIFFS_CREAT  | SPIFFS_WRONLY , 0);
		if(m_fd < 0) {
			SPIFFS_close(&SYSFS, m_fd);
			PRINTF("[nest command bftp] init: failed to rewrite file %s\n", m_concat_buffer);
			return ENOENT;
		}
	}
	return ENONE;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_bftp_init_process, ev, data)
{
	static nest_command_data_t *input;
	static uint32_t err_code = ENONE;
	static uint32_t process_data_len;
	static uint32_t timeout_type;
	PROCESS_BEGIN();

	input = data;
#if DEBUG_MODULE > 1
	PRINTF("[nest command bftp] start init process\n");
	for (uint8_t i = 0; i < input->len; i++) {
		PRINTF("%2x ", input->data[i]);
	}
	PRINTF("\n");
#endif

	clear_output_data();

	m_output.opcode = input->opcode;
	m_output.response_process = input->response_process;

#if defined(USE_WDUI_STACK)
	if (scr_launcher_get_actions() >= SCR_LAUNCHER_MAX_ACTION) {
		if (m_bftp_operate_type & 0x80) {
			m_output.len = 0;
		} else {
			err_code = ENOMEM;
			m_output.data[0] = err_code;
			m_output.len = 1;
		}
		PRINTF("[nest command bftp] init: exceed actions max\n");
		goto response;
	}
#endif

	// Return busy
	if (m_bftp_processing || lunchr_is_loading()) {
		if (m_bftp_operate_type & 0x80) {
			m_output.len = 0;
		} else {
			err_code = EBUSY;
			m_output.data[0] = err_code;
			m_output.len = 1;
		}
		PRINTF("[nest command bftp] init: busy, processing\n");
		goto response;
	}

	if (input->len != 13 && input->len != 9) {
		if (m_bftp_operate_type & 0x80) {
			m_output.len = 0;
		} else {
			err_code = EINVAL;
			m_output.data[0] = err_code;
			m_output.len = 1;
		}
		PRINTF("[nest command bftp] init: input length %d\n", input->len);
		goto response;
	}

	m_bftp_operate_type = input->data[0];

	// copy the file uuid (4 bytes)
	m_file_uuid = ((uint32_t)input->data[1] << 24) | ((uint32_t)input->data[2] << 16) |
				((uint32_t)input->data[3] << 8) | ((uint32_t)input->data[4]);

#if defined(LUNCHR_SINGLETON_APP_MODE)
	m_file_uuid = LUNCHR_SINGLETON_APP_FILE_UUID;
#endif

	// copy the file size (4 bytes)
	m_file_size = ((uint32_t)input->data[5] << 24) | ((uint32_t)input->data[6] << 16) |
				((uint32_t)input->data[7] << 8) | ((uint32_t)input->data[8]);

	// Non-used variable
	m_file_last_packets = m_file_size % DATA_PACKETS_MAX;
	m_file_total_notify_num =  m_file_size / DATA_PACKETS_MAX;
	if (m_file_last_packets != 0)
		m_file_total_notify_num += 1;
	m_file_wr_process_size_num = 0;

	// Read operation
	if (m_bftp_operate_type & 0x80) {

		// estiblish open file and reply data
		err_code = bftp_init_read_file(&m_output.data[0], &process_data_len);
		if (err_code != ENONE) {
			m_output.len = 0;
			PRINTF("[nest command bftp] init: read file %d\n", err_code);
			goto response;
		}
		m_output.len = process_data_len;

		m_bftp_processing = true;
		m_file_checksum = (m_file_checksum ^ calculate_xor( m_output.data, m_output.len) );
		m_file_wr_process_size = m_output.len;
		m_retries = 0;
		m_total_retries = 0;

		timeout_type = BFTP_TIMEOUT_SET_INIT;
		process_post_synch(&bftp_timeout_process, PROCESS_EVENT_CONTINUE, timeout_type);

#if defined(USE_WDUI_STACK)
		wdui_nest_event_notify(NEST_BFTP_INIT, NULL);
#endif

	} else {

		// Make sure cap size if lower then total fs
		// if (m_file_size > 40000){
		// 	err_code = EINVAL;
		// 	m_output.data[0] = err_code;
		// 	m_output.len = 1;
		// 	PRINTF("[nest command bftp] init: file size %d\n", m_file_size);
		// 	goto response;
		// }
		// Check size
		u32_t total, used;
		SPIFFS_info(&SYSFS, &total, &used);
		if ((m_file_size) >= (total - used)){
			PRINTF("[nest command bftp] storage not enough: need: %d internal flash less:%d\n", (m_file_size), (total - used));
			err_code = EISCA;
			m_output.data[0] = err_code;
			m_output.len = 1;
			goto response;
		}

		SPIFFS_gc(&SYSFS, m_file_size);
		// include ext uuid and data
		if (input->len == 13) {
			// copy the ext-file uuid (4 bytes)
			m_ext_file_uuid = ((uint32_t)input->data[9] << 24) | ((uint32_t)input->data[10] << 16) |
						((uint32_t)input->data[11] << 8) | ((uint32_t)input->data[12]);

			PRINTF("[nest command bftp] init ext file: uuid:%08x, file size %d, ext uuid:%08x\n",
				m_file_uuid, m_file_size, m_ext_file_uuid);

		} else if (input->len == 9) {
			PRINTF("[nest command bftp] init file: uuid:%08x, file size %d\n",
				m_file_uuid, m_file_size);
		}

		err_code = bftp_init_write_file();
		if (err_code != ENONE) {
			m_output.data[0] = err_code;
			m_output.len = 1;
			PRINTF("[nest command bftp] init: write file %d\n", err_code);
			goto response;
		}

		// start initiate the process
		m_bftp_processing = true;
		m_file_checksum = 0;
		m_file_wr_process_size = 0;
		m_file_wr_process_size_num = 0;
		m_retries = 0;
		m_total_retries = 0;
		memset(batch, 0x00, BATCHSIZE);
		batch_num = 0;

		// Request data
		m_output.data[0] = ENONE;

		// copy offset bytes (start from zero)
		m_output.data[1] = 0;
		m_output.data[2] = 0;
		m_output.data[3] = 0;
		m_output.data[4] = 0;

		// copy needs length.
		if(m_file_size > 16)
			process_data_len = 16;
		else
			process_data_len = m_file_size;

		m_output.data[5] = (process_data_len & 0x000000FF);
		m_output.data[6] = (process_data_len & 0x0000FF00) >> 8;
		m_output.data[7] = (process_data_len & 0x00FF0000) >> 16;
		m_output.data[8] = (process_data_len & 0xFF000000) >> 24;

		m_output.len = 9;

		timeout_type = BFTP_TIMEOUT_SET_INIT;
		process_post_synch(&bftp_timeout_process, PROCESS_EVENT_CONTINUE, timeout_type);

#if defined(USE_WDUI_STACK)
		wdui_nest_event_notify(NEST_BFTP_INIT, NULL);
#endif

	}
response:
	nest_command_send(&m_output);
  // process_start(&nest_command_response_process, (void *)m_output)
	// if (process_is_running(&nest_command_response_process)) {
	// 	PROCESS_WAIT_EVENT_UNTIL( (ev == PROCESS_EVENT_EXITED && data == &nest_command_response_process));
	//
	// }
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_bftp_packets_process, ev, data)
{
	static nest_command_data_t *input;
	static uint32_t err_code;
	static uint32_t process_data_len;
	static uint32_t percent;
	static uint32_t timeout_type;
	PROCESS_BEGIN();

	input = data;
#if DEBUG_MODULE > 1
	//PRINTF("nest command bftp_packets process\n");
	for (uint8_t i = 0; i < input->len; i++) {
		PRINTF("%2x ", input->data[i]);
	}
	PRINTF("\n");
#endif

	clear_output_data();

	m_output.opcode = input->opcode;
	m_output.response_process = input->response_process;

	if (!m_bftp_processing ) {
		if (m_bftp_operate_type & 0x80) {
			m_output.len = 0;
		} else {
			m_output.data[0] = EINVALSTATE;
			m_output.len = 1;
		}
		PRINTF("[nest command bftp] packets invild stage\n");
		goto response;
	}

	if (m_bftp_operate_type & 0x80) {
		// copy the file size (4 bytes)
		uint32_t offset = ((uint32_t)input->data[0] << 24) | ((uint32_t)input->data[1] << 16) |
					((uint32_t)input->data[2] << 8) | ((uint32_t)input->data[3]);

		uint32_t request_size = ((uint32_t)input->data[4] << 24) | ((uint32_t)input->data[5] << 16) |
					((uint32_t)input->data[6] << 8) | ((uint32_t)input->data[7]);


		PRINTF("[nest command bftp] read offset %d size %d\n", offset, request_size);

		int ret;
		if (is_file_type_prefer_stored_external()) {
			ret = SPIFFS_lseek(&SFS, m_fd, offset, SPIFFS_SEEK_SET);
			if (ret < 0){
				PRINTF("[nest command bftp] read lseek error %i\n", SPIFFS_errno(&SFS));
				SPIFFS_close(&SFS, m_fd);
				m_output.len = 0;
				goto response;
			}
		  ret = SPIFFS_read(&SFS, m_fd, &m_output.data[0], request_size);
			if (ret < 0){
				PRINTF("[nest command bftp] read error %i\n", SPIFFS_errno(&SFS));
				SPIFFS_close(&SFS, m_fd);
				m_output.len = 0;
				goto response;
			}
		} else {
			ret = SPIFFS_lseek(&SYSFS, m_fd, offset, SPIFFS_SEEK_SET);
			if (ret < 0){
				PRINTF("[nest command bftp] read lseek error %i\n", SPIFFS_errno(&SYSFS));
				SPIFFS_close(&SYSFS, m_fd);
				m_output.len = 0;
				timeout_type = BFTP_TIMEOUT_STOP;
				process_post_synch(&bftp_timeout_process, PROCESS_EVENT_CONTINUE, timeout_type);
				goto response;
			}
		  ret = SPIFFS_read(&SYSFS, m_fd, &m_output.data[0], request_size);
			if (ret < 0){
				PRINTF("[nest command bftp] read error %i\n", SPIFFS_errno(&SYSFS));
				SPIFFS_close(&SYSFS, m_fd);
				m_output.len = 0;
				timeout_type = BFTP_TIMEOUT_STOP;
				process_post_synch(&bftp_timeout_process, PROCESS_EVENT_CONTINUE, timeout_type);
				goto response;
			}

			m_output.len = request_size;
			m_file_wr_process_size += request_size;
			m_file_checksum = (m_file_checksum ^ calculate_xor( m_output.data, m_output.len) );
			m_retries = 0;
			m_file_wr_process_size_num++;

			timeout_type = BFTP_TIMEOUT_SET_PACKETS;
			process_post_synch(&bftp_timeout_process, PROCESS_EVENT_CONTINUE, timeout_type);

#if defined(USE_WDUI_STACK)
			percent = m_file_wr_process_size_num * 100 / m_file_total_notify_num;
			wdui_nest_event_notify(NEST_BFTP_PROCESS, percent);
#endif

		}

	} else {
		// update process size
		m_file_wr_process_size += input->len;
		m_file_checksum = (m_file_checksum ^ calculate_xor( input->data, input->len) );
		memcpy(&batch[batch_num], input->data, input->len);
		batch_num += input->len;
		if (batch_num >= BATCHSIZE ) {
			int ret;
			if (is_file_type_prefer_stored_external()) {
				ret = SPIFFS_write(&SFS, m_fd, batch, batch_num);
			} else {
			  ret = SPIFFS_write(&SYSFS, m_fd, batch, batch_num);
			}

			if (ret < 0){
				PRINTF("[nest command bftp] append faild errno %i\n", SPIFFS_errno(&SYSFS));
				SPIFFS_close(&SYSFS, m_fd);
				m_output.data[0] = EAPPEN;
				m_output.len = 1;
				goto response;
			}
			batch_num = 0;
			memset(batch, 0x00, BATCHSIZE);
		}

		m_output.data[0] = ENONE;

		// Set image offset.
		m_output.data[1] = (m_file_wr_process_size & 0x000000FF);
		m_output.data[2] = (m_file_wr_process_size & 0x0000FF00) >> 8;
		m_output.data[3] = (m_file_wr_process_size & 0x00FF0000) >> 16;
		m_output.data[4] = (m_file_wr_process_size & 0xFF000000) >> 24;

		process_data_len = m_file_size - m_file_wr_process_size;
		if(process_data_len > 16)
			process_data_len = 16;

		// PRINTF("[nest command bftp] total %d recv %d, needs %d\n", m_file_size,
		// 																								m_file_wr_process_size, process_data_len);
		m_output.data[5] = (process_data_len & 0x000000FF);
		m_output.data[6] = (process_data_len & 0x0000FF00) >> 8;
		m_output.data[7] = (process_data_len & 0x00FF0000) >> 16;
		m_output.data[8] = (process_data_len & 0xFF000000) >> 24;
		m_output.len = 9;

		m_retries = 0;
		m_file_wr_process_size_num++;

		timeout_type = BFTP_TIMEOUT_SET_PACKETS;
		process_post_synch(&bftp_timeout_process, PROCESS_EVENT_CONTINUE, timeout_type);

#if defined(USE_WDUI_STACK)
		percent = m_file_wr_process_size_num * 100 / m_file_total_notify_num;
		wdui_nest_event_notify(NEST_BFTP_PROCESS, percent);
#endif

	}
response:
	nest_command_send(&m_output);
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_bftp_end_process, ev, data)
{
	static nest_command_data_t *input;
	static int fd, r;
	static uint32_t appsize, err_code;
	static uint8_t checksum;
	static uint32_t timeout_type;
	PROCESS_BEGIN();

	input = data;
#if DEBUG_MODULE > 1
	PRINTF("[nest command bftp] end process\n");
#endif

	clear_output_data();

	m_output.opcode = input->opcode;
	m_output.response_process = input->response_process;

	if (!m_bftp_processing) {
		m_output.data[0] = EINVALSTATE;
		m_output.len = 1;
		PRINTF("[nest command bftp] end: invild stage\n");
		goto response;
	}

	// checksum (4 bytes) : reserve (12 bytes)
	if (input->len != 4) {
		m_output.data[0] = EINVAL;
		m_output.len = 1;
		if (is_file_type_prefer_stored_external()) {
			SPIFFS_close(&SFS, m_fd);
		} else {
			SPIFFS_close(&SYSFS, m_fd);
		}
		PRINTF("[nest command bftp] end: wrong params\n");
		goto response;
	}

  checksum = input->data[0];
	PRINTF("[nest command bftp] end packet success, total retries %d, total process:%d, checksum:0x%02x \n",
								m_total_retries, m_file_wr_process_size, m_file_checksum);

	if (m_file_wr_process_size != m_file_size) {
		// error : miss data
		PRINTF("[nest command bftp] end received %d total %d, miss data\n", m_file_wr_process_size, m_file_size);
		m_output.data[0] = EDSNM;
		m_output.len = 1;
		if (is_file_type_prefer_stored_external()) {
			SPIFFS_close(&SFS, m_fd);
		} else {
			SPIFFS_close(&SYSFS, m_fd);
		}
		goto response;

	}	else if (m_file_checksum != checksum) {
		// error : checksum errorlunchr_exam_app
		PRINTF("[nest command bftp] end checksum error, need %d get %d\n",
																								m_file_checksum, checksum);
		m_output.data[0] = EBADRQC;
		m_output.len = 1;
		if (is_file_type_prefer_stored_external()) {
			SPIFFS_close(&SFS, m_fd);
		} else {
			SPIFFS_close(&SYSFS, m_fd);
		}
		goto response;

	} else {
		// final check
		// err_code = lunchr_exam_app(m_file_uuid);
		// if (err_code != ENONE) {
		// 	m_output.data[0] = err_code;
		// 	m_output.len = 1;
		// 	goto response;
		// }
		if (m_bftp_operate_type & 0x80) {
			PRINTF("[nest command bftp] end read\n");
			if (is_file_type_prefer_stored_external()) {
				SPIFFS_close(&SFS, m_fd);
			} else {
				SPIFFS_close(&SYSFS, m_fd);
			}
		} else {
			if (batch_num != 0 ) {
				int ret = SPIFFS_write(&SYSFS, m_fd, batch, batch_num);
				if (ret < 0){
					PRINTF("[lunchr] failed to flush batch %i\n", SPIFFS_errno(&SYSFS));
					if (is_file_type_prefer_stored_external()) {
						SPIFFS_close(&SFS, m_fd);
					} else {
						SPIFFS_close(&SYSFS, m_fd);
					}
					m_output.data[0] = EAPPEN;
					m_output.len = 1;

					goto response;
				}
				batch_num = 0;
				memset(batch, 0x00, BATCHSIZE);
			}
		}
		if (is_file_type_prefer_stored_external()) {
			SPIFFS_close(&SFS, m_fd);
		} else {
			SPIFFS_close(&SYSFS, m_fd);
		}
	}

	m_bftp_processing = false;
	m_file_wr_process_size = 0;
	m_file_checksum = 0;
	m_total_retries = 0;
	m_file_wr_process_size_num = 0;
  m_file_total_notify_num = 0;
	m_file_last_packets = 0;

	m_output.data[0] = ENONE;
	m_output.len = 1;

#if defined(USE_WDUI_STACK)
	wdui_nest_event_notify(NEST_BFTP_END, NULL);
#endif

response:
  // ctimer_stop(&m_bftp_timeout_timer);
	timeout_type = BFTP_TIMEOUT_STOP;
	process_post_synch(&bftp_timeout_process, PROCESS_EVENT_CONTINUE, timeout_type);
	nest_command_send(&m_output);
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_bftp_remove_process, ev, data)
{
	static nest_command_data_t *input;
	static char *appname;
	static uint32_t err_code;
	static uint32_t total, used;
	PROCESS_BEGIN();

	input = data;

#if DEBUG_MODULE > 1
	PRINTF("nest command bftp_remove process\n");
	for (uint8_t i = 0; i < input->len; i++) {
		PRINTF("%2x ", input->data[i]);
	}
	PRINTF("\n");
#endif

	clear_output_data();

	m_output.opcode = input->opcode;
	m_output.response_process = input->response_process;

	// uuid (4 bytes) + reserve (12 bytes)
	if (input->len != 4) {
		PRINTF("[nest command bftp] wrong params\n");
		m_output.data[0] = EINVAL;
		m_output.len = 1;
		goto response;
	}

	m_file_uuid = ((uint32_t)input->data[0] << 24) | ((uint32_t)input->data[1] << 16) |
				((uint32_t)input->data[2] << 8) | ((uint32_t)input->data[3]);

	// err_code = lunchr_remove_app(m_file_uuid);

	// remove internal file
	sprintf(m_concat_buffer, "a/%08x", m_file_uuid);
	SPIFFS_remove(&SYSFS, m_concat_buffer);
	sprintf(m_concat_buffer, "n/%08x", m_file_uuid);
	SPIFFS_remove(&SYSFS, m_concat_buffer);
	sprintf(m_concat_buffer, "i/%08x", m_file_uuid);
	SPIFFS_remove(&SYSFS, m_concat_buffer);
	sprintf(m_concat_buffer, "t/%08x", m_file_uuid);
	SPIFFS_remove(&SYSFS, m_concat_buffer);
	sprintf(m_concat_buffer, "p/%08x", m_file_uuid);
	SPIFFS_remove(&SYSFS, m_concat_buffer);

	int res;
	spiffs_DIR d;
	struct spiffs_dirent e;
	struct spiffs_dirent *pe = &e;
	spiffs_file fd = -1;
	sprintf(m_concat_ext_buffer, "s/%08x", m_file_uuid);
	m_concat_ext_buffer[10] = '/';
	char *search_prefix = m_concat_ext_buffer;
	SPIFFS_opendir(&SFS, "/", &d);
	while ((pe = SPIFFS_readdir(&d, pe))) {
		if (0 == strncmp(search_prefix, (char *)pe->name, 11)) {
			// found one
			fd = SPIFFS_open_by_dirent(&SFS, pe, SPIFFS_RDWR, 0);
			if (fd < 0) {
				PRINTF("[nest command bftp] open dirent errno %i\n", SPIFFS_errno(&SFS));
				SPIFFS_closedir(&d);
				m_output.data[0] = EINTERNAL;
				m_output.len = 1;
				goto response;
			}
			res = SPIFFS_fremove(&SFS, fd);
			if (res < 0) {
				PRINTF("[nest command bftp] fremove errno %i\n", SPIFFS_errno(&SFS));
				SPIFFS_closedir(&d);
				m_output.data[0] = EINTERNAL;
				m_output.len = 1;
				goto response;
			}
			res = SPIFFS_close(&SFS, fd);
			if (res < 0) {
				PRINTF("[nest command bftp] close errno %i\n", SPIFFS_errno(&SFS));
				SPIFFS_closedir(&d);
				m_output.data[0] = EINTERNAL;
				m_output.len = 1;
				goto response;
			}
		}
	}
	SPIFFS_closedir(&d);

	SPIFFS_info(&SYSFS, &total, &used);
	int r = SPIFFS_gc(&SYSFS, total - used);
	PRINTF("[nest lunchr bftp] SPIFFS gc ret %d\n", r);


#if defined(USE_WDUI_STACK)
	wdui_nest_event_notify(NEST_BFTP_REMOVE, NULL);
#endif

	m_output.data[0] = ENONE;
	m_output.len = 1;

response:
	nest_command_send(&m_output);
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_bftp_stat_process, ev, data)
{
	static nest_command_data_t *input;
	static int res;
	static spiffs_stat s;
	static spiffs_file fd;
	static bool is_type_supported;
	static uint32_t file_size;
	PROCESS_BEGIN();

	input = data;
#if DEBUG_MODULE > 1
	PRINTF("[nest command bftp] stat process\n");
	for (uint8_t i = 0; i < input->len; i++) {
		PRINTF("%2x ", input->data[i]);
	}
	PRINTF("\n");
#endif

	clear_output_data();

	m_output.opcode = input->opcode;
	m_output.response_process = input->response_process;

	// type (1 byte) + uuid (4 bytes)
	if (input->len != 5) {
		PRINTF("[nest command bftp] stat wrong params\n");
		m_output.data[0] = EINVAL;
		m_output.len = 1;
		goto response;
	}

	m_bftp_operate_type = input->data[0];

	// copy the file uuid (4 bytes)
	m_file_uuid = ((uint32_t)input->data[1] << 24) | ((uint32_t)input->data[2] << 16) |
				((uint32_t)input->data[3] << 8) | ((uint32_t)input->data[4]);

	is_type_supported = check_and_setup_concat_buffer();
	if (!is_type_supported) {
		PRINTF("[nest command bftp] stat type no support\n");
		m_output.data[0] = ENOSUPPORT;
		m_output.len = 1;
		goto response;
	}

	if (is_file_type_prefer_stored_external()) {
		res = SPIFFS_stat(&SFS, m_concat_buffer, &s);
	} else {
		res = SPIFFS_stat(&SYSFS, m_concat_buffer, &s);
	}

	if (res == SPIFFS_ERR_NOT_FOUND) {
		m_output.data[0] = ENOFOUND;
		m_output.len = 1;
		goto response;
	}

	file_size = s.size;

	// Set image offset.
	m_output.data[1] = (file_size & 0x000000FF);
	m_output.data[2] = (file_size & 0x0000FF00) >> 8;
	m_output.data[3] = (file_size & 0x00FF0000) >> 16;
	m_output.data[4] = (file_size & 0xFF000000) >> 24;

	m_output.data[0] = ENONE;
	m_output.len = 5;

response:
	nest_command_send(&m_output);
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_bftp_space_used_process, ev, data)
{
	static nest_command_data_t *input;
	static uint32_t total, used;
	static uint32_t ext_total, ext_used;
	PROCESS_BEGIN();

	input = data;
#if DEBUG_MODULE > 1
	PRINTF("nest command bftp_space_used process\n");
	for (uint8_t i = 0; i < input->len; i++) {
		PRINTF("%2x ", input->data[i]);
	}
	PRINTF("\n");
#endif

	clear_output_data();

	m_output.opcode = input->opcode;
	m_output.response_process = input->response_process;

#if STORAGE_SYSC_INTERNAL_CONF == 1
	SPIFFS_info(&SYSFS, &total, &used);
	ext_total = 0;
	ext_used = 0;
#else
	SPIFFS_info(&SYSFS, &total, &used);
	SPIFFS_info(&SFS, &ext_total, &ext_used);
#endif

PRINTF("[nest command bftp] space used, total %d, used %d, ext total %d used %d\n",
						total, used, ext_total, ext_used);

	m_output.data[0] = (total & 0x000000FF);
	m_output.data[1] = (total & 0x0000FF00) >> 8;
	m_output.data[2] = (total & 0x00FF0000) >> 16;
	m_output.data[3] = (total & 0xFF000000) >> 24;

	m_output.data[4] = (used & 0x000000FF);
	m_output.data[5] = (used & 0x0000FF00) >> 8;
	m_output.data[6] = (used & 0x00FF0000) >> 16;
	m_output.data[7] = (used & 0xFF000000) >> 24;

	m_output.data[8] = (ext_total & 0x000000FF);
	m_output.data[9] = (ext_total & 0x0000FF00) >> 8;
	m_output.data[10] = (ext_total & 0x00FF0000) >> 16;
	m_output.data[11] = (ext_total & 0xFF000000) >> 24;

	m_output.data[12] = (ext_used & 0x000000FF);
	m_output.data[13] = (ext_used & 0x0000FF00) >> 8;
	m_output.data[14] = (ext_used & 0x00FF0000) >> 16;
	m_output.data[15] = (ext_used & 0xFF000000) >> 24;
	m_output.len = 16;
	nest_command_send(&m_output);
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(nest_bftp_readdir_process, ev, data)
{
	static nest_command_data_t *input;
	static int res, n;
	static spiffs_stat s;
	static spiffs_file fd;
	static bool is_type_supported;
	static uint32_t file_size;
	static char *search_prefix;
	static uint32_t file_idx;
	static uint32_t total_file_size;
	static char uuid_hexstr[8];
	PROCESS_BEGIN();

	input = data;
#if DEBUG_MODULE > 1
	PRINTF("[nest command bftp] stat process\n");
	for (uint8_t i = 0; i < input->len; i++) {
		PRINTF("%2x ", input->data[i]);
	}
	PRINTF("\n");
#endif

	clear_output_data();

	m_output.opcode = input->opcode;
	m_output.response_process = input->response_process;

	// type (1 byte)
	if (input->len != 1) {
		PRINTF("[nest command bftp] readdir wrong params\n");
		m_output.data[0] = EINVAL;
		m_output.len = 1;
		goto response;
	}

	m_bftp_operate_type = input->data[0];

	is_type_supported = check_and_setup_concat_buffer();
	if (!is_type_supported) {
		PRINTF("[nest command bftp] readdir type no support\n");
		m_output.data[0] = ENOSUPPORT;
		m_output.len = 1;
		goto response;
	}

	spiffs_DIR d;
	struct spiffs_dirent e;
	struct spiffs_dirent *pe = &e;
	search_prefix = get_prefix(m_bftp_operate_type);
	SPIFFS_opendir(&SYSFS, "/", &d);
	file_idx = 0;
	total_file_size = 0;
	while ((pe = SPIFFS_readdir(&d, pe))) {
		if (0 == strncmp(search_prefix, (char *)pe->name, 2)) {
			PRINTF("[nest command bftp] readdir %s, size %d\n", pe->name, pe->size);
			// Copy the file name
			memcpy(uuid_hexstr, &pe->name[2], 8);

			m_output.data[0] = ENONE;

			// Copy the uuid (Hex String) to 1~4
			for(int i = 0; i < 4; i++) {
				sscanf(uuid_hexstr+2*i, "%2X", &n);
				m_output.data[i + 1] = (char)n;
			}

			// Copy the size to 5~8
			m_output.data[5] = (pe->size & 0x000000FF);
			m_output.data[6] = (pe->size & 0x0000FF00) >> 8;
			m_output.data[7] = (pe->size & 0x00FF0000) >> 16;
			m_output.data[8] = (pe->size & 0xFF000000) >> 24;

			// Copy the index (Hex String) to 9~12
			m_output.data[9] = (file_idx & 0x000000FF);
			m_output.data[10] = (file_idx & 0x0000FF00) >> 8;
			m_output.data[11] = (file_idx & 0x00FF0000) >> 16;
			m_output.data[12] = (file_idx & 0xFF000000) >> 24;
			file_idx++;
			total_file_size += pe->size;

			m_output.len = 13;
			nest_command_send(&m_output);
		}
	}
	SPIFFS_closedir(&d);

	PRINTF("[nest command bftp] readdir total file %d\n", file_idx);

	// complete: total file size
	m_output.data[0] = ENONE;
	m_output.data[1] = (total_file_size & 0x000000FF);
	m_output.data[2] = (total_file_size & 0x0000FF00) >> 8;
	m_output.data[3] = (total_file_size & 0x00FF0000) >> 16;
	m_output.data[4] = (total_file_size & 0xFF000000) >> 24;
	m_output.len = 5;

response:
	nest_command_send(&m_output);
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(bftp_timeout_process, ev, data)
{
	static uint32_t type;
	static struct ctimer ct;
	PROCESS_BEGIN();
	while(1) {
		PROCESS_WAIT_EVENT_UNTIL (ev == PROCESS_EVENT_CONTINUE);
		type = data;
		if (type == BFTP_TIMEOUT_SET_INIT) {
			PRINTF("[nest command bftp] set init timeout\n");
			ctimer_set(&ct, NEST_COMMAND_BFTP_SEND_TIMEOUT , bftp_init_timer_callback_handler, (void *)NULL);
		} else if (type == BFTP_TIMEOUT_SET_PACKETS) {
			PRINTF("[nest command bftp] set packets timeout\n");
			ctimer_set(&ct, NEST_COMMAND_BFTP_SEND_TIMEOUT , bftp_packtes_timer_callback_handler, (void *)NULL);
		} else if (type == BFTP_TIMEOUT_STOP) {
			PRINTF("[nest command bftp] stop timeout\n");
			ctimer_stop(&ct);
		}
	}
	PROCESS_END();
}

#endif

/*---------------------------------------------------------------------------*/
void
nest_command_bftp_init(void)
{
#if NEST_COMMAND_BFTP_ENABLE_CONF == 1
	nest_command_register(&bftp_init_command);
	nest_command_register(&bftp_packets_command);
	nest_command_register(&bftp_end_command);
	nest_command_register(&bftp_remove_command);
	nest_command_register(&bftp_stat_command);
	nest_command_register(&bftp_space_used_command);
	nest_command_register(&bftp_readdir_command);

	if (!process_is_running(&bftp_timeout_process))
		process_start(&bftp_timeout_process, NULL);
#endif

}
/*---------------------------------------------------------------------------*/
bool
nest_command_bftp_is_processing(void)
{
#if NEST_COMMAND_BFTP_ENABLE_CONF == 1
	return m_bftp_processing;
#else
	return false;
#endif
}
/*---------------------------------------------------------------------------*/
void
nest_command_bftp_reset(void)
{
#if NEST_COMMAND_BFTP_ENABLE_CONF == 1
	uint32_t timeout_type = BFTP_TIMEOUT_STOP;
	process_post_synch(&bftp_timeout_process, PROCESS_EVENT_CONTINUE, timeout_type);
	m_retries = 0;
	m_bftp_processing = false;
	m_file_wr_process_size = 0;
	m_file_checksum = 0;
	m_total_retries = 0;
	m_file_wr_process_size_num = 0;
	m_file_total_notify_num = 0;
	m_file_last_packets = 0;
	m_file_wr_process_size_num = 0;
	SPIFFS_close(&SYSFS, m_fd);
	PRINTF("[nest command bftp] stop retries\n");
#endif
}
/*---------------------------------------------------------------------------*/
