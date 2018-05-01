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
#ifndef _APP_SYSCALL_H_
#define _APP_SYSCALL_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

enum {
	APP_LIFECYCLE_TERMINATE = 0x00
};

struct app_lifecycle_event {
  struct app_lifecycle_event *next;
	char 												*name;
  void 												(*terminating)(void);
};

int app_lifecycle_register(struct app_lifecycle_event *events);
void app_lifecycle_notify(uint8_t opcode);
void app_lifecycle_init(void);

#ifdef __cplusplus
}
#endif
#endif /* _APP_SYSCALL_H_ */
