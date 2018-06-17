/**
 * © Copyright AlfaLoop Technology Co., Ltd. 2018
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/
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
