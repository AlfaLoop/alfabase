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
	int (*write)(const void *buf, uint32_t len, uint32_t offset);
	int (*read)(void *buf, uint32_t len, uint32_t offset);
	int (*subscribe)(void *buf, uint32_t len, HWCallbackHandler handler);
	int (*close)(void *args);
} HWDriver;

HWDriver* HWPipe(const char *dev);
HWDriver* HWGet(uint32_t idx);
int HWNum(void);
/* Framework API */

/* Back-end */
void hw_api_init(void);
int hw_api_bsp_num(void);
HWDriver* hw_api_bsp_get(uint32_t idx);
HWDriver* hw_api_bsp_pipe(const char *dev);
/* Back-end */


#ifdef __cplusplus
}
#endif
#endif /* _HW_API_H */
