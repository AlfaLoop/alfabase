/**
 *  Copyright (c) 2016-2018 AlfaLoop Technology Co., Ltd. All Rights Reserved.
 */
#ifndef _NEST_SERIAL_H_
#define _NEST_SERIAL_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "contiki.h"
#include <stdint.h>
#include <stdbool.h>

#define NEST_SERIAL_HEADER1    0xA5
#define NEST_SERIAL_HEADER2    0x5A

// 0xA5 5A len data checksum
void nest_serial_init(void);

// this function is interrupt safe
void nest_serial_input(uint8_t input);


/**
 * The structure of a AT-command bsp implementation
 */
struct nest_serial_driver {
	void (* open)(void);
  void (* close)(void);
  int (* send)(char *s, int len);
};

PROCESS_NAME(nest_serial_process);
PROCESS_NAME(nest_serial_send_process);


/**
 * Deprecated platform-specific routines.
 */
void nest_serial_bsp_enable(void);
void nest_serial_bsp_disable(void);
void nest_serial_bsp_send(uint8_t *data, uint32_t len);

#ifdef __cplusplus
}
#endif
#endif /* _NEST_SERIAL_H_ */
