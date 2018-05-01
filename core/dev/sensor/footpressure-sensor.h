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
#ifndef __FOOTPRESSURE_SENSOR_H_
#define __FOOTPRESSURE_SENSOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define DEV_FOOTPRESSURE_LEFT     0x00;
#define DEV_FOOTPRESSURE_RIGHT    0x01;

typedef void (* footpressure_raw_data_func_t)(int16_t thumb, int16_t outter_ball,
  int16_t inner_ball, int16_t heel, uint32_t timestamp);


typedef struct _footpressure_config_t {
  footpressure_raw_data_func_t        framework_raw_data_source;
} footpressure_config_t;

/**
 * The structure of a FoorPressure sensor in Contiki.
 */
struct footpressure_driver {
	char *name;
	int (* init)(footpressure_config_t *config);
	int (* poweron)(void);
	int (* poweroff)(void);
  bool (* activated)(void);
  uint8_t (* get_type)(void);
  int16_t (* get_thumb)(void);
  int16_t (* get_outer_ball)(void);
  int16_t (* get_inner_ball)(void);
  int16_t (* get_heel)(void);
};

#ifdef __cplusplus
}
#endif
#endif /* __FOOTPRESSURE_SENSOR_H_ */
