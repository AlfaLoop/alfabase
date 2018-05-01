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
#ifndef _UART_H
#define _UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef void (* rx_cb)(uint8_t data);

typedef struct
{
  uint32_t tx;
  uint32_t rx;
  uint32_t cts;
  uint32_t rts;
  uint32_t baudrate;
  bool hwfc;
	rx_cb cb;
} uart_config_t;

/**
 * The structure of a UART driver
 */
struct uart_driver {
	char *name;

	/** Initialize the UART driver */
	int (* init)(uart_config_t *config);

	/** Disable the Uart driver */
	int (* disable)(void);

	/** TX with config */
	int (* tx_with_config)(uart_config_t *config, uint8_t *data, uint32_t length);

	/** TX */
	int (* tx)(uint8_t *data, uint32_t length);
};



#ifdef __cplusplus
}
#endif
#endif /* _UART_H */
