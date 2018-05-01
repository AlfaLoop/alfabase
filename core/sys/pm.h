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
