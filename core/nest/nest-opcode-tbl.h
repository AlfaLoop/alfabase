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
#ifndef _NEST_OPCODE_TBL_H_
#define _NEST_OPCODE_TBL_H_
#ifdef __cplusplus
extern "C" {
#endif

enum
{
	nest_air_opcode_none = 0x00,

	nest_air_opcode_bftp_init = 0x01,
	nest_air_opcode_bftp_packets = 0x02,
	nest_air_opcode_bftp_end = 0x03,
	nest_air_opcode_bftp_remove = 0x04,
	nest_air_opcode_bftp_stat = 0x05,
	nest_air_opcode_bftp_space_used = 0x06,
	nest_air_opcode_bftp_readdir = 0x07,

	nest_air_opcode_lunchr_exec = 0x10,
	nest_air_opcode_lunchr_kill = 0x11,
	nest_air_opcode_lunchr_set_boot_process = 0x12,
	nest_air_opcode_lunchr_remove_boot_process = 0x13,
	nest_air_opcode_lunchr_running_query = 0x14,
	nest_air_opcode_lunchr_boot_query = 0x15,
	nest_air_opcode_lunchr_hardfault_kill = 0x16,

	nest_air_opcode_core_version = 0x20,			 /* TODO */
	nest_air_opcode_core_uuid = 0x21,				 /* TODO */
	nest_air_opcode_core_platform = 0x22,			 /* TODO */
	nest_air_opcode_core_battery_query = 0x23,
	nest_air_opcode_core_switch_bootloader = 0x24,
	nest_air_opcode_core_channel_disconnect = 0x25,
	nest_air_opcode_core_time_sync = 0x26,

	nest_air_opcode_pipe = 0x30,
	nest_air_opcode_pipe_airlog = 0x31,

	nest_air_opcode_host_enter_background = 0x40,	 /* TODO */
	nest_air_opcode_host_enter_forground = 0x41,	 /* TODO */
	nest_air_opcode_host_sleep = 0x42,				 /* TODO */
	nest_air_opcode_host_platform_ios =  0x43,		 /* TODO */
	nest_air_opcode_host_platform_andriod =  0x44,	 /* TODO */
	nest_air_opcode_host_platform_wp =  0x45,		 /* TODO */

	nest_air_opcode_auth_echo = 0x50,
	nest_air_opcode_auth_challenge = 0x51,			 /* TODO */
	nest_air_opcode_auth_chipid = 0x52,				 /* TODO */
	nest_air_opcode_auth_otpt = 0x53,				 /* TODO */
	nest_air_opcode_auth_reverse = 0x54,

	nest_air_opcode_dfu_init = 0x60,			 		/* TODO */
	nest_air_opcode_dfu_verify = 0x61,				 /* TODO */
	nest_air_opcode_dfu_write = 0x62,			 		/* TODO */
	nest_air_opcode_dfu_active_n_reset = 0x63,  /* TODO */
	nest_air_opcode_dfu_resp_statu = 0x64		/* TODO */
};

// Reply type
enum
{
	nest_air_reply_none = 0x00,
	nest_air_reply_ota_init_suc = 0x01,
	nest_air_reply_ota_init_error = 0x02,
	nest_air_reply_ota_packet_suc = 0x03,
	nest_air_reply_ota_packet_error = 0x04,
	nest_air_reply_ota_end_suc = 0x05,
	nest_air_reply_ota_end_error = 0x06,
	nest_air_reply_ota_exec_suc = 0x07,
	nest_air_reply_ota_exec_error = 0x08,
	nest_air_reply_ota_verify = 0x09,
	nest_air_reply_auth_reverse = 0x20,
	nest_air_reply_err = 0xff
};

// Reply type (ota list)
enum
{
	nest_air_err_ota_none = 0x00,
	nest_air_err_ota_miss_data = 0x01,
	nest_air_err_ota_checksum = 0x02,
	nest_air_err_ota_internal = 0x03,
};

// Reply type (exec error list)
enum
{
	nest_air_err_exec_none = 0x00,
	nest_air_err_exec_bad_elfloader = 0x01,
	nest_air_err_exec_no_symtab	= 0x02,
	nest_air_err_exec_no_strtab	= 0x03,
	nest_air_err_exec_no_text		= 0x04,
	nest_air_err_exec_symbol_not_found = 0x05,
	nest_air_err_exec_segment_not_found = 0x06,
	nest_air_err_exec_no_startpoint = 0x07
};

#ifdef __cplusplus
}
#endif
#endif // _NEST_OPCODE_TBL_H_
