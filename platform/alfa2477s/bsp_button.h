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
#ifndef ___BSP_BUTTON_H
#define ___BSP_BUTTON_H
#ifdef __cplusplus
extern "C" {
#endif

#include "contiki.h"
#include "frameworks/hw/hw_api.h"

int bsp_button_init(void);
int bsp_button_open(void *args);
int bsp_button_write(const void *buf, uint32_t len, uint32_t offset);
int bsp_button_read(void *buf, uint32_t len, uint32_t offset);
int bsp_button_close(void *args);

#ifdef __cplusplus
}
#endif
#endif /* ___BSP_BUTTON_H */
