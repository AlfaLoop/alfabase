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
#ifndef _HW_UART_API_H
#define _HW_UART_API_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Framework API */
enum {
	BAUDRATE_1200 = 1200,
  BAUDRATE_2400 = 2400,
  BAUDRATE_4800 = 4800,
  BAUDRATE_9600 = 9600,
	BAUDRATE_19200 = 19200,
  BAUDRATE_38400 = 28400,
  BAUDRATE_57600 = 57600,
  BAUDRATE_115200 = 115200,
};

typedef void (* UartRxHandler)(uint8_t data);

typedef struct {
	int (* open)(uint32_t pin_tx, uint32_t pin_rx, uint32_t baudrate, UartRxHandler handler);
  int (* close)(void);
  int (* send)(uint8_t *data, uint32_t length);
} Uart;
/* Framework API */

/* Back-end */
Uart* bsp_hw_uart_api_retrieve(uint8_t number);
void hw_uart_terminating(void);
void hw_data_source(uint8_t data);
/* Back-end */

#ifdef __cplusplus
}
#endif
#endif /* _HW_UART_API_H */
