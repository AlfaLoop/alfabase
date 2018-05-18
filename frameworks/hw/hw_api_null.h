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
#ifndef _HW_API_NULL_H
#define _HW_API_NULL_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "hw_api.h"
#include "hw_uart_api.h"

int hw_null_open(void *args);
int hw_null_write(const void *buf, uint32_t len, uint32_t *offset);
int hw_null_read(void *buf, uint32_t len, uint32_t offset);
int hw_null_subscribe(void *buf, uint32_t len, HWCallbackHandler handler);
int hw_null_close(void *args);

int null_p_2_uint32_r_int(uint32_t pin, uint8_t value);
int null_p_1_uint32_r_int(uint32_t pin);

int null_pinInfo(uint8_t *pin, uint8_t *size);
int null_pin_read(uint32_t pin, uint8_t *value);
int null_pin_watch_set(uint32_t pin_mask, uint32_t edge_mask, GpioEventHandler handler);
int null_pin_watch_close(void);

int null_uart_open(uint32_t pin_tx, uint32_t pin_rx, uint32_t baudrate, UartRxHandler handler);
int null_uart_close(void);
int null_uart_send(uint8_t *data, uint32_t length);

int null_i2c_init(uint32_t sda_pin, uint32_t scl_pin, uint32_t speed);
int null_i2c_read(uint32_t address, uint8_t *value);
int null_i2c_write(uint32_t address, uint8_t *value);
int null_i2c_close(void);

int null_spi_init(uint32_t mosi_pin, uint32_t miso_pin, uint32_t sclk_pin, uint32_t speed);
int null_spi_read(uint32_t address, uint8_t *value);
int null_spi_write(uint32_t address, uint8_t *value);
int null_spi_close(void);

#ifdef __cplusplus
}
#endif
#endif /* _HW_API_NULL_H */
