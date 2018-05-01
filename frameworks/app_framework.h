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
#ifndef _APP_FRAMEWORK_H_
#define _APP_FRAMEWORK_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "contiki.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Framework API */
typedef int (*OSProcessEntry_t)(void);
int OSProcessEntrySetup(OSProcessEntry_t p_entry);
/* Framework API */

/* Back-end API */
int app_framework_init(void);
int app_framework_create_task(void);
int app_framework_remove_task(void);
int app_framework_task_suspend(void);
bool app_framework_is_app_running(void);
/* Back-end API */

#ifdef __cplusplus
}
#endif
#endif /* _APP_FRAMEWORK_H_ */
