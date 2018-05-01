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
#ifndef _MOTOR_H_
#define _MOTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifdef USE_MOTOR

int motor_start(void);
int motor_stop(void);

#else

#define power_status(attr)

#endif

#ifdef __cplusplus
}
#endif
#endif /* _MOTOR_H_ */
