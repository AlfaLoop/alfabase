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
#include "init.h"
#include "contiki.h"
#include "spiffs/spiffs.h"
#include "loader/lunchr.h"
#include "sys/bootloader.h"
#include "nest/nest.h"
#include "sys/systime.h"
#include "sys/pm.h"
#ifdef USE_ELFLOADER
#include "loader/elfloader.h"
#endif
// #ifdef USE_FRAMEWORK
// #include "frameworks/core/osfile_api.h"

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
static sys_bsp_init_func bsp_callback = NULL;
/*---------------------------------------------------------------------------*/
#if defined(USE_FRAMEWORK)
static void
load_default_boot_app(void)
{
	uint32_t uuid;
	uint32_t err_code;
	err_code = lunchr_get_boot_task_uuid(&uuid);
	if (err_code == ENONE) {
		PRINTF("[init] Boot %08x\n", uuid);
#if defined(USE_WDUI_STACK)
		// err_code = wdui_switch_screen(SCR_EXTENSION, m_uuid);
#else
		if (lunchr_load_app_with_uuid(uuid) != ENONE) {
			PRINTF("[init] load boot file failed: remove file\n");
			lunchr_remove_boot_task();
		}
#endif
	} else if (err_code == ENOENT) {
		PRINTF("[init] no boot file\n");
	} else if (err_code == EINTERNAL) {
		PRINTF("[init] load boot file failed\n");
	}
}
#endif
/*---------------------------------------------------------------------------*/
static void
exam_filesystem(void)
{
	// Check the first initialization of files system ( for file system formatting)
	char buf[12];
	int cfd;
	spiffs_file fd;
	int r;
	u32_t total, used;

	// Check internal filesystem
	SPIFFS_info(&SYSFS, &total, &used);
	PRINTF("[init] internal flash total:%d used:%d\n", total, used);

	spiffs_DIR d;
	struct spiffs_dirent e;
	struct spiffs_dirent *pe = &e;

	SPIFFS_opendir(&SYSFS, "/", &d);

	while ((pe = SPIFFS_readdir(&d, pe))) {
		PRINTF("[init] %s [%04x] size:%i\n", pe->name, pe->obj_id, pe->size);
	}
	SPIFFS_closedir(&d);

	fd = SPIFFS_open(&SYSFS, ".init", SPIFFS_RDWR, 0);
	if (SPIFFS_read(&SYSFS, fd, (u8_t *)buf, 12) < 0){
		PRINTF("[init] errno %i\n", SPIFFS_errno(&SYSFS));
		SPIFFS_close(&SYSFS, fd);
		PRINTF("[init] No .init file found, creating!\n");
		fd = SPIFFS_open(&SYSFS, ".init", SPIFFS_CREAT | SPIFFS_RDWR, 0);
		if (SPIFFS_write(&SYSFS, fd, (u8_t *)"Hello world", 12) < 0){
			PRINTF("[init] errno %i\n", SPIFFS_errno(&SYSFS));
		}
		SPIFFS_close(&SYSFS, fd);
	}
	else {
		PRINTF("[init] .init exist\n");
		SPIFFS_close(&SYSFS, fd);
	}

#ifdef USE_SPI_FLASH
	total = 0;
	used = 0;
	SPIFFS_info(&SFS, &total, &used);
	PRINTF("[init] ext flash total:%d used:%d\n", total, used);
	spiffs_DIR ed;
	struct spiffs_dirent ee;
	struct spiffs_dirent *pee = &ee;
	SPIFFS_opendir(&SFS, "/", &ed);
	while ((pee = SPIFFS_readdir(&ed, pee))) {
		PRINTF("[init] %s [%04x] size:%i\n", pee->name, pee->obj_id, pee->size);
	}
	SPIFFS_closedir(&d);
#endif

	//SPIFFS_vis(&SFS);
}
/*---------------------------------------------------------------------------*/
PROCESS(sys_init_process, "sys_init_process");
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(sys_init_process, ev, data)
{
	static struct etimer et;
	PROCESS_BEGIN();
	etimer_set(&et, 500);
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

	if (bsp_callback != NULL)
		bsp_callback();

	// exam the filesystem
	exam_filesystem();

	// init system time
	systime_init();

	// load the default application
#if defined(USE_ELFLOADER)
	load_default_boot_app();
#endif

	// show the system version
	PRINTF("[init] %s Patch:%02x%02x\n", ALFABASE_VERSION_STRING, NEST_PLATFORM_VERSION_H_CONF, NEST_PLATFORM_VERSION_L_CONF);
	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
sys_init(sys_bsp_init_func func)
{
	bsp_callback = func;
  // setup a callback function to check filesystem
	if (!process_is_running(&sys_init_process))
		process_start(&sys_init_process, NULL);
}
/*---------------------------------------------------------------------------*/
