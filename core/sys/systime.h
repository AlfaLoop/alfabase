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
