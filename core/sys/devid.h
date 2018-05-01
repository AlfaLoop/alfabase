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
#ifndef _DEV_ID_H_
#define _DEV_ID_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

int devid_gen(uint8_t *id, int max_len);
int hwid_gen(uint8_t *id, int max_len);
int devcode_gen(uint8_t *id);
int devid_reboot(void);
int devid_bootloader_mode(void);

#ifdef __cplusplus
}
#endif
#endif /* _DEV_ID_H_ */
