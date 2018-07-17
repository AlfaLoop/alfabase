/**
 *  Copyright (c) 2016 AlfaLoop, Inc. All Rights Reserved.
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
#ifndef _UART_ARCH_H
#define _UART_ARCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "dev/uart.h"

extern const struct uart_driver uart0;

#ifdef __cplusplus
}
#endif
#endif /* _UART_ARCH_H */