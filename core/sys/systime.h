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
#ifndef __SYSTIME_H_
#define __SYSTIME_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "contiki.h"

typedef struct _systime_period_event {
  struct _systime_period_event *next;
  void 											(*callback)(void);
} systime_period_event_t;

uint32_t systime_in_seconds(void);
uint32_t systime_hour_24(void);
uint32_t systime_hour_12(void);
uint32_t systime_hour_12_designator(void);
uint32_t systime_minutes(void);
uint32_t systime_seconds(void);
uint32_t systime_day_of_week(void);
uint32_t systime_day_of_month(void);
uint32_t systime_day_of_year(void);
uint32_t systime_month(void);
uint32_t systime_year(void);
uint32_t systime_epoch_time(void);

int systime_period_event_register(systime_period_event_t *event);
int systime_set_current_time(uint32_t t);
void systime_update_current_time(uint32_t year, uint32_t month, uint32_t day, uint32_t hour, uint32_t minute);
void systime_init(void);
// void systime_adjust_arch(void);

PROCESS_NAME(systime_process);

#ifdef __cplusplus
}
#endif
#endif /* __SYSTIME_H_ */
