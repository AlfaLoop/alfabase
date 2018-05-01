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
#ifndef __HEARTRATE_SENSOR_H_
#define __HEARTRATE_SENSOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define HRM_DATA_TYPE_RAWDATA   0x01
#define HRM_DATA_TYPE_BPM       0x02

typedef void (* hrm_raw_data_func_t)(uint8_t type, uint32_t value);
typedef struct _heartrate_config_t {
  hrm_raw_data_func_t        framework_raw_data_source;
} heartrate_config_t;

/**
 * The structure of a Accelerometer sensor in Contiki.
 */
struct heartrate_driver {
	char *name;
	int (* init)(heartrate_config_t *config);
	int (* poweron)(void);
	int (* poweroff)(void);
  bool (* is_active)(void);
  int (* touch)(void);
  uint8_t (* get_last_bpm)(void);
};

#ifdef __cplusplus
}
#endif
#endif /* __HEARTRATE_SENSOR_H_ */
