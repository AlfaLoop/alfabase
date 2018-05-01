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
#ifndef _I2C_H_
#define _I2C_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define I2C_TX 		 			0x00
#define I2C_RX  				0x01

#define I2C_PEDDING				0x01
#define I2C_DONT_PEDDING		0x00

#define I2C_ISSUE_STOP 			0x01
#define I2C_DONT_ISSUE_STOP 	0x00

typedef struct {
  uint32_t sda;
  uint32_t scl;
  uint32_t speed;
} i2c_config_t;

/**
 * The structure of a i2c in Contiki.
 */
struct i2c_driver {
	char *name;

	/** Initialize the I2C driver */
	int (* init)(const i2c_config_t *config);

	/** Enable the I2C driver */
	int (* enable)(void);

	/** Disable the I2C driver */
	int (* disable)(void);

	/** Data transfer */
	int (* transfer)(uint8_t type, uint8_t address, uint32_t length, uint8_t * p_data, uint8_t pending);
};



#ifdef __cplusplus
}
#endif
#endif /* _I2C_H_ */
