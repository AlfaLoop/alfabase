/**
 *  Copyright (c) 2016 AlfaLoop Technology Co., Ltd. All Rights Reserved.
 *
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential.
 *
 *  Attribution — You must give appropriate credit, provide a link to the license, and
 *  indicate if changes were made. You may do so in any reasonable manner, but not in any
 *  way that suggests the licensor endorses you or your use.
 *
 *  NonCommercial — You may not use the material for commercial purposes under unauthorized.
 *
 *  NoDerivatives — If you remix, transform, or build upon the material, you may not
 *  distribute the modified material.
 */
#ifndef _I2C_SW1_H_
#define _I2C_SW1_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "dev/i2c.h"

int i2csw1_init(const i2c_config_t *config);
int i2csw1_enable(void);
int i2csw1_disable(void);
int i2csw1_transfer(uint8_t type, uint8_t address, uint32_t length, uint8_t * p_data, uint8_t pending);


#ifdef __cplusplus
}
#endif
#endif /* _I2C_SW1_H_ */
