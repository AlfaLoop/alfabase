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
#ifndef _PM_H_
#define _PM_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "bsp_init.h"

#ifndef PM_CONF_DRIVER
#error "Need define PM_CONF_DRIVER in bsp_init.h"
#else
#define PM PM_CONF_DRIVER
#endif /* PM_CONF_DRIVER */

#define PM_SOURCE_NONE       0x00
#define PM_SOURCE_BATTERY    0x01
#define PM_SOURCE_CHARGING    0x02

typedef enum {
  PM_SAVING_MODE = 0x00000000,
  PM_CHARGING_MODE,
  PM_NORMAL_MODE,
} pm_mode_t;

void pm_init(void);
uint8_t pm_get_battery(void);
uint32_t pm_current_mode(void);
void pm_switch_mode(uint32_t mode);

/**
 * The structure of a Power (battery) in Contiki.
 */
struct pm_driver {
	char *name;
	uint8_t (* get_battery_level)(void);
  uint8_t (* get_charging_status)(void);
};

extern const struct pm_driver PM;

#ifdef __cplusplus
}
#endif
#endif /* _PM_H_ */
