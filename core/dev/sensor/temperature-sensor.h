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
#ifndef _TEMPERATURE_SENSOR_H_
#define _TEMPERATURE_SENSOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * The structure of a Baseband RF driver (BTLE) in Contiki.
 */
struct temperature_driver {
	char *name;

	/** Initialize the temperature driver */
	int (* init)(void);

	/** Enable the temperature driver */
	int (* poweron)(void);

	/** Disable the temperature driver */
	int (* poweroff)(void);

	/** Get temperature level data */
	int (* data)(uint32_t *p_data);
};

#ifdef __cplusplus
}
#endif
#endif /* _TEMPERATURE_SENSOR_H_ */
