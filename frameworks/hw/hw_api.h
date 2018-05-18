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
#ifndef _HW_API_H
#define _HW_API_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "hw_gpio_api.h"
#include "hw_i2c_api.h"
#include "hw_spi_api.h"
#include "hw_uart_api.h"

/* Framework API */
typedef void (* HWCallbackHandler)(void *args);

typedef struct{
	char *name;
	int (*open)(void *args);
	int (*write)(const void *buf, uint32_t len, uint32_t *offset);
	int (*read)(void *buf, uint32_t len, uint32_t offset);
	int (*subscribe)(void *buf, uint32_t len, HWCallbackHandler handler);
	int (*close)(void *args);
} HWDriver;

HWDriver* HWPipe(const char *dev);
HWDriver* HWGet(uint32_t idx);
int HWNum(void);

Gpio* HWGpio(void);
Uart* HWUart(uint8_t number);
I2c* HWI2c(uint8_t number);
Spi* HWSpi(uint8_t number);
/* Framework API */

/* Back-end */
#define HW_GPIO    0x0001
#define HW_UART    0x0002
#define HW_I2C     0x0004
#define HW_SPI     0x0008
#define HW_PWM     0x0010
#define HW_ADC     0x0020
#define HW_I2S     0x0040

typedef struct {
	uint8_t pin;
	uint16_t mode;
} hw_pin_mode_t;

typedef void (* hw_api_bsp_terminating_callback)(void);

void hw_api_init(void);
bool hw_api_check_pin(uint32_t pin, uint16_t type);
void hw_api_bsp_init(const hw_pin_mode_t *pins, uint8_t num, hw_api_bsp_terminating_callback callback);

int hw_api_bsp_num(void);
HWDriver* hw_api_bsp_get(uint32_t idx);
HWDriver* hw_api_bsp_pipe(const char *dev);
/* Back-end */


#ifdef __cplusplus
}
#endif
#endif /* _HW_API_H */
