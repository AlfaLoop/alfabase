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
#ifndef _OSTIMER_API_H
#define _OSTIMER_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "contiki.h"

/* Framework API */
#define OSTIMER_MAX_NUMBER  3
typedef void (* OSTimerHandler)(uint8_t id);

typedef enum {
  TIMER0 = 0x00,
  TIMER1 = 0x01,
  TIMER2 = 0x02
} TimerId;

typedef struct{
  int (*start)(TimerId id, uint16_t milliseconds, OSTimerHandler handler);
  int (*stop)(TimerId id);
} Timer;

Timer* OSTimer(void);
/* Framework API */

/* Back-end */
PROCESS_NAME(ostimer_0_api);
PROCESS_NAME(ostimer_1_api);
PROCESS_NAME(ostimer_2_api);
process_event_t ostimer_event_timeout;

void ostimer_api_init(void);
/* Back-end */


#ifdef __cplusplus
}
#endif
#endif /* _OSTIMER_API_H */
