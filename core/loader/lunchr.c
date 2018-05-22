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
#include "lunchr.h"
#include "errno.h"
#include "libs/util/xor.h"
#include "sys/clock.h"
#include "dev/syslog.h"
#include "spiffs/spiffs.h"
#include "sys/bootloader.h"
#include "nest/nest.h"
#include "bsp_init.h"

#ifdef USE_ELFLOADER
#include "loader/symtab.h"
#include "loader/symbol-std.h"
#include "loader/elfloader.h"
#endif
/*---------------------------------------------------------------------------*/
#if defined(DEBUG_ENABLE)
#define DEBUG_MODULE 1
#if DEBUG_MODULE
#define PRINTF(...) syslog(__VA_ARGS__)
#else
#define PRINTF(...)
#endif  /* DEBUG_MODULE */
#else
#define PRINTF(...)
#endif  /* DEBUG_ENABLE */
/*---------------------------------------------------------------------------*/
static uint8_t m_app_loading = 0;
static uint32_t m_running_app_uuid = 0;
static char m_concat_buffer[ELF_CONCAT_BUFFER_SIZE];
static spiffs_file m_lunchr_fd;
/*---------------------------------------------------------------------------*/
static int
lunchr_load_app(char *dirname)
{
	int fd, ret, i;
	char *print, *symbol;
	uint32_t err_code = ENONE;

	uint32_t start_time = clock_time();

#ifdef USE_ELFLOADER
	fd = SPIFFS_open(&SYSFS, dirname, SPIFFS_RDWR, 0);
	if(fd < 0) {
		// error : checksum error
		PRINTF("[lunchr] failed open %i\n", SPIFFS_errno(&SYSFS));
		err_code = ENOENT;
		SPIFFS_close(&SYSFS, fd);
	} else {

		elfloader_init_process();
		ret = elfloader_load(fd);
		SPIFFS_close(&SYSFS, fd);

		symbol = "";
		switch(ret) {
		case ELFLOADER_OK:
			print = "OK";
			err_code = ENONE;
			break;
		case ELFLOADER_BAD_ELF_HEADER:
			print = "Bad ELF header";
			err_code = EBEH;
			break;
		case ELFLOADER_NO_SYMTAB:
			print = "No symbol table";
			err_code = ENSYMT;
			break;
		case ELFLOADER_NO_STRTAB:
			print = "No string table";
			err_code = ENSTRT;
			break;
		case ELFLOADER_NO_TEXT:
			print = "No text segment";
			err_code = ENTXTSG;
			break;
		case ELFLOADER_SYMBOL_NOT_FOUND:
			print = "Symbol not found: ";
			symbol = elfloader_unknown;
			err_code = ESYMNF;
			break;
		case ELFLOADER_SEGMENT_NOT_FOUND:
			print = "Segment not found: ";
			symbol = elfloader_unknown;
			err_code = ESGNF;
			break;
		case ELFLOADER_NO_STARTPOINT:
			print = "No starting point";
			err_code = ENSTARTP;
			break;
		default:
			print = "Unknown return code from the ELF loader (internal bug)";
			err_code = EINTERNAL;
			break;
		}

		if( ret == ELFLOADER_OK) {
			 PRINTF("[lunchr] exec: %s consume %d\n", print, clock_time() - start_time);
			// check photothread
			for(i = 0; elfloader_autostart_processes[i] != NULL; ++i) {
				//PRINTF("[lunchr] exec: starting process '%s'\n", elfloader_autostart_processes[1]->name);
				if ( strcmp(elfloader_autostart_processes[i]->name, "up1") == 0 ) {
					// start contiki process: process start call user main entry
					// PRINTF("[lunchr] exec: starting up1 %08x\n", uuid);
					autostart_start(elfloader_autostart_processes);
					err_code = ENONE;
				} else {
					PRINTF("[lunchr] exec: open %s\n", dirname);
					SPIFFS_remove(&SYSFS, dirname);
					m_running_app_uuid = 0;
					err_code = ENOENT;
				}
			}
		}	else {
			PRINTF("[lunchr] %s %s, remove: %s\n", print, symbol, dirname);
			m_running_app_uuid = 0;
			SPIFFS_remove(&SYSFS, dirname);
		}
	}
#else
	// TODO: MicroPython VM

#endif
	return err_code;
}
/*---------------------------------------------------------------------------*/
int
lunchr_get_running_task_uuid(uint32_t *p_uuid)
{
#ifdef USE_FRAMEWORK
	if (!app_framework_is_app_running())
		return EINVALSTATE;
#else
	return ENOSUPPORT;
#endif

	*p_uuid = m_running_app_uuid;
	return ENONE;
}
/*---------------------------------------------------------------------------*/
int
lunchr_get_boot_task_uuid(uint32_t *p_uuid)
{
	int r, n;
	static char uuid_hexstr[8];
	uint8_t u[4];
	uint32_t uuid;
	spiffs_file fd;

	fd = SPIFFS_open(&SYSFS, LUNCHR_BOOT_TASK_NAME, SPIFFS_RDWR, 0);
	if (fd < 0) {
		PRINTF("[lunchr] %s file error %i\n", LUNCHR_BOOT_TASK_NAME, SPIFFS_errno(&SYSFS));
		SPIFFS_close(&SYSFS, fd);
		return ENOENT;
	} else {
		r = SPIFFS_read(&SYSFS, fd, (u8_t *)uuid_hexstr, 8);
		// convert uuid_hexstr to uuid
		if (r < 0) {
			// PRINTF("[lunchr] Can't read .boot\n");
			SPIFFS_close(&SYSFS, fd);
			return EINTERNAL;
		} else {
			for(int i = 0; i < 4; i++) {
				sscanf(uuid_hexstr+2*i, "%2X", &n);
				u[i] = (char)n;
			}
			uuid = (uint32_t)(u[0] << 24);
			uuid |= (uint32_t)(u[1] << 16);
			uuid |= (uint32_t)(u[2] << 8);
			uuid |= (uint32_t)(u[3]);
			SPIFFS_close(&SYSFS, fd);
			*p_uuid = uuid;
		}
	}
	return ENONE;
}
/*---------------------------------------------------------------------------*/
bool
lunchr_is_running(void)
{
#ifdef USE_FRAMEWORK
	if(app_framework_is_app_running())
		return true;
#endif
	return false;
}
/*---------------------------------------------------------------------------*/
bool
lunchr_is_loading(void)
{
	if (m_app_loading) {
		return 1;
	}
	return 0;
}
/*---------------------------------------------------------------------------*/
int
lunchr_load_app_with_uuid(uint32_t uuid)
{
	char ft[3];
	spiffs_file fd;
	int ret = ENONE;

#ifdef USE_FRAMEWORK
	if (app_framework_is_app_running()) {
		return EBUSY;
	}
#endif

	if (m_app_loading) {
		return EBUSY;
	}

	// store the uuid
	m_running_app_uuid = uuid;

	sprintf(m_concat_buffer, "a/%08x", uuid);
	PRINTF("[lunchr] load elf with uuid %s\n", m_concat_buffer);
	m_app_loading = 1;
	ret = lunchr_load_app(m_concat_buffer);
	m_app_loading = 0;
	return ret;
}
/*---------------------------------------------------------------------------*/
int
lunchr_kill_running_app(void)
{
	uint32_t errcode;
#ifdef USE_FRAMEWORK
	if (app_framework_is_app_running()) {
		errcode = app_framework_remove_task();
		if (errcode == ENONE) {
			PRINTF("[lunchr] user process kick out! \n");
			if (nest_central_status == NEST_CENTRAL_STATUS_NONE) {
				NEST.reset();
			}
			return ENONE;
		}
	}
	PRINTF("[lunchr] no user process running! \n");
	return EINVALSTATE;
#else
	// TODO: micropython vm

	return ENONE;
#endif
}
/*---------------------------------------------------------------------------*/
int
lunchr_set_boot_task(uint32_t *p_uuid)
{
	int fd, ret;
	char uuid[8];
#ifdef USE_FRAMEWORK
	if (!app_framework_is_app_running())
		return EINVALSTATE;
#endif

  fd = SPIFFS_open(&SYSFS, LUNCHR_BOOT_TASK_NAME, SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_WRONLY, 0);
	if(fd < 0) {
		// error : checksum error
		// PRINTF("[lunchr] set auto load: failed to open %i\n", SPIFFS_errno(&SYSFS));
		SPIFFS_close(&SYSFS, fd);
		return ENULLP;
	} else {
		// save the uuid
		*p_uuid = m_running_app_uuid;
		sprintf(uuid, "%08x", m_running_app_uuid);
		ret = SPIFFS_write(&SYSFS, fd, uuid, 8);
		if ( ret < 0 ){
			// PRINTF("[lunchr] Can't create .boot\n");
			SPIFFS_close(&SYSFS, fd);
			return EINTERNAL;
		}
	}

  SPIFFS_close(&SYSFS, fd);
	return ENONE;
}
/*---------------------------------------------------------------------------*/
int
lunchr_remove_boot_task(void)
{
  SPIFFS_remove(&SYSFS, LUNCHR_BOOT_TASK_NAME);
	return ENONE;
}
/*---------------------------------------------------------------------------*/
