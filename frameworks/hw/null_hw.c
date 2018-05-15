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
#include "null_hw.h"
#include "frameworks/app_lifecycle.h"
#include "frameworks/app_eventpool.h"
#include "loader/symtab.h"
#include "errno.h"
/*---------------------------------------------------------------------------*/
#if defined(DEBUG_ENABLE)
#define DEBUG_MODULE 0
#if DEBUG_MODULE
#include "dev/syslog.h"
#define PRINTF(...) syslog(__VA_ARGS__)
#else
#define PRINTF(...)
#endif  /* DEBUG_MODULE */
#else
#define PRINTF(...)
#endif  /* DEBUG_ENABLE */
/*---------------------------------------------------------------------------*/
int
null_p_2_uint32_r_int(uint32_t pin, uint8_t value)
{
  return ENOSUPPORT;
}
/*---------------------------------------------------------------------------*/
int
null_p_1_uint32_r_int(uint32_t pin)
{
  return ENOSUPPORT;
}
/*---------------------------------------------------------------------------*/
int
null_pin_read(uint32_t pin, uint8_t *value)
{
  return ENOSUPPORT;
}
/*---------------------------------------------------------------------------*/
int
null_pinInfo(uint8_t *pin, uint8_t *size)
{
  return ENOSUPPORT;
}
/*---------------------------------------------------------------------------*/
int
null_pin_watch_set(uint32_t pin_mask, uint32_t edge_mask, GpioEventHandler handler)
{
  return ENOSUPPORT;
}
/*---------------------------------------------------------------------------*/
int
null_pin_watch_close(void)
{
  return ENOSUPPORT;
}
/*---------------------------------------------------------------------------*/
int
null_uart_open(uint32_t pin_tx, uint32_t pin_rx, uint32_t baudrate, UartRxHandler handler)
{
  return ENOSUPPORT;
}
/*---------------------------------------------------------------------------*/
int
null_uart_close(void)
{
  return ENOSUPPORT;
}
/*---------------------------------------------------------------------------*/
int
null_uart_send(uint8_t *data, uint32_t length)
{
  return ENOSUPPORT;
}
/*---------------------------------------------------------------------------*/
int
null_i2c_init(uint32_t sda_pin, uint32_t scl_pin, uint32_t speed)
{
  return ENOSUPPORT;
}
/*---------------------------------------------------------------------------*/
int
null_i2c_read(uint32_t address, uint8_t *value)
{
  return ENOSUPPORT;
}
/*---------------------------------------------------------------------------*/
int
null_i2c_write(uint32_t address, uint8_t *value)
{
  return ENOSUPPORT;
}
/*---------------------------------------------------------------------------*/
int
null_i2c_close(void)
{
  return ENOSUPPORT;
}
/*---------------------------------------------------------------------------*/
int
null_spi_init(uint32_t mosi_pin, uint32_t miso_pin, uint32_t sclk_pin, uint32_t speed)
{
  return ENOSUPPORT;
}
/*---------------------------------------------------------------------------*/
int
null_spi_read(uint32_t address, uint8_t *value)
{
  return ENOSUPPORT;
}
/*---------------------------------------------------------------------------*/
int
null_spi_write(uint32_t address, uint8_t *value)
{
  return ENOSUPPORT;
}
/*---------------------------------------------------------------------------*/
int
null_spi_close(void)
{
  return ENOSUPPORT;
}
/*---------------------------------------------------------------------------*/
