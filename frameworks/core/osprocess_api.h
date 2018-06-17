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
#ifndef _OSPROCESS_API_H
#define _OSPROCESS_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

/* Framework API */
typedef struct {
  void (*onEnterBackground)(void);
  void (*onEnterForeground)(void);
  void (*onWillTerminate)(void);
} OSProcessCallback;

typedef struct{
  int (*waitForEvent)(void);
  uint32_t (*getUuid)(void);
  uint32_t (*getVersion)(void);
  uint32_t (*getClockTick)(void);
  void (*delay)(uint16_t millis);
  void (*delayUs)(uint16_t microseconds);
  int (*getEnv)(void *params, int size);
} Process;

// Singleton instance of OSLog
Process* OSProcess(void);
int OSProcessEventAttach(const OSProcessCallback *callback);
int OSProcessEventDetach(void);
/* Framework API */

/* Back-end */
#define OSPROCESS_EVENT_TYPE_TERMINATE      0x01
#define OSPROCESS_EVENT_TYPE_BACKGROUND     0x02
#define OSPROCESS_EVENT_TYPE_FOREGROUND     0x03

void osprocess_api_init(void);
void osprocess_api_terminate(void);
void osprocess_api_background(void);
void osprocess_api_foreground(void);
bool osprocess_app_in_background(void);
/* Back-end */


#ifdef __cplusplus
}
#endif
#endif /* _OSPROCESS_API_H */
