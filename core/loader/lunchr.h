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
