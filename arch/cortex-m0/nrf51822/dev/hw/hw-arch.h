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
#ifndef _HW_ARCH_H
#define _HW_ARCH_H
#ifdef __cplusplus
extern "C" {
#endif

#include "frameworks/hw/hw_api.h"

int hw_api_arch_init(const uint8_t *enabled_pin_list, uint32_t size);

// HW API
int hw_arch_pinSet(uint32_t pin, uint8_t value);
int hw_arch_pinRead(uint32_t pin, uint8_t *value);
int hw_arch_pinInfo(uint8_t *pin, uint8_t *size);
int hw_arch_pinWatchSet(uint32_t pin_mask, uint32_t edge_mask, PinWatchtHandler handler);
int hw_arch_pinWatchClose(void);
int hw_arch_uartInit(uint32_t pin_tx, uint32_t pin_rx, uint32_t baudrate, UartRxHandler handler);
int hw_arch_uartDisable(void);
int hw_arch_uartSend(uint8_t *data, uint32_t length);

#ifdef __cplusplus
}
#endif
#endif /* _HW_ARCH_H */
