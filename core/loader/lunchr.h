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
#ifndef _LUNCHR_H_
#define _LUNCHR_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// System only maintance 1 app
#if LUNCHR_SINGLETON_APP_CONF == true
	#define LUNCHR_SINGLETON_APP_MODE 			1
	#define LUNCHR_SINGLETON_APP_FILE_UUID  50
#endif

#define LUNCHR_NAME_MAX_LENGTH      16

#define LUNCHR_APP_PREFIX            "a/"
#define LUNCHR_FILE_TYPE_PREFIX      "t/"
#define LUNCHR_FILE_PREFIX           "s/"
#define LUNCHR_ICON_PREFIX           "i/"
#define LUNCHR_NAME_PREFIX           "n/"

#define LUNCHR_SINGLETON_APP_UUID   ".uuid"
#define LUNCHR_BOOT_TASK_NAME  			".boot"

#define ELF_CONCAT_BUFFER_SIZE		10

// int lunchr_load_app_with_dirname(char *dirname);
int lunchr_load_app_with_uuid(uint32_t uuid);
int lunchr_kill_running_app(void);
int lunchr_set_boot_task(uint32_t *p_uuid);
int lunchr_remove_boot_task(void);

int lunchr_get_running_task_uuid(uint32_t *p_uuid);
int lunchr_get_boot_task_uuid(uint32_t *p_uuid);

bool lunchr_is_running(void);
bool lunchr_is_loading(void);
//int lunchr_app_exit(void);

int lunchr_tolnum_of_app(void);
int lunchr_retrive_app(uint32_t idx, uint32_t *uuid, char *shortname);

#ifdef __cplusplus
}
#endif
#endif /* _LUNCHR_H_ */
