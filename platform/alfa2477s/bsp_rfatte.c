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
#include "bsp_rfatte.h"
#include <stdint.h>
#include <stdbool.h>
#include "frameworks/hw/hw_api.h"
#include "frameworks/hw/hw_api_null.h"
#include "frameworks/app_eventpool.h"
#include "frameworks/app_lifecycle.h"
#include "nrf_gpio.h"
#include "errno.h"
#include "bsp_init.h"
/*---------------------------------------------------------------------------*/
#if defined(DEBUG_ENABLE)
#define DEBUG_MODULE 1
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
#define RADIO_ATT_16dbm          RADIO_PIN_0
#define RADIO_ATT_8dbm           RADIO_PIN_1
#define RADIO_ATT_4dbm           RADIO_PIN_2
#define RADIO_ATT_2dbm           RADIO_PIN_3
#define RADIO_ATT_1dbm           RADIO_PIN_4
enum {
  RADIO_0dBm = 0x00,
  RADIO_1dBm,
  RADIO_2dBm,
  RADIO_4dBm,
  RADIO_8dBm,
  RADIO_16dBm,
  RADIO_31dBm,
};
/*---------------------------------------------------------------------------*/
static uint8_t curr_attenuator = RADIO_0dBm;
/*---------------------------------------------------------------------------*/
static void
set_attenuator(uint8_t att)
{
  PRINTF("[bsp rfatte] set attenuator ");
  switch (att) {
    case 0x00:
    nrf_gpio_pin_set(RADIO_ATT_1dbm);
    nrf_gpio_pin_set(RADIO_ATT_2dbm);
    nrf_gpio_pin_set(RADIO_ATT_4dbm);
    nrf_gpio_pin_set(RADIO_ATT_8dbm);
    nrf_gpio_pin_set(RADIO_ATT_16dbm);
    curr_attenuator = RADIO_0dBm;
    PRINTF("0dbm\n");
    break;
    case 0x01:
    nrf_gpio_pin_clear(RADIO_ATT_1dbm);
    nrf_gpio_pin_set(RADIO_ATT_2dbm);
    nrf_gpio_pin_set(RADIO_ATT_4dbm);
    nrf_gpio_pin_set(RADIO_ATT_8dbm);
    nrf_gpio_pin_set(RADIO_ATT_16dbm);
    curr_attenuator = RADIO_1dBm;
    PRINTF("1dbm\n");
    break;
    case 0x02:
    nrf_gpio_pin_set(RADIO_ATT_1dbm);
    nrf_gpio_pin_clear(RADIO_ATT_2dbm);
    nrf_gpio_pin_set(RADIO_ATT_4dbm);
    nrf_gpio_pin_set(RADIO_ATT_8dbm);
    nrf_gpio_pin_set(RADIO_ATT_16dbm);
    curr_attenuator = RADIO_2dBm;
    PRINTF("2dbm\n");
    break;
    case 0x03:
    nrf_gpio_pin_set(RADIO_ATT_1dbm);
    nrf_gpio_pin_set(RADIO_ATT_2dbm);
    nrf_gpio_pin_clear(RADIO_ATT_4dbm);
    nrf_gpio_pin_set(RADIO_ATT_8dbm);
    nrf_gpio_pin_set(RADIO_ATT_16dbm);
    curr_attenuator = RADIO_4dBm;
    PRINTF("4dbm\n");
    break;
    case 0x04:
    nrf_gpio_pin_set(RADIO_ATT_1dbm);
    nrf_gpio_pin_set(RADIO_ATT_2dbm);
    nrf_gpio_pin_set(RADIO_ATT_4dbm);
    nrf_gpio_pin_clear(RADIO_ATT_8dbm);
    nrf_gpio_pin_set(RADIO_ATT_16dbm);
    curr_attenuator = RADIO_8dBm;
    PRINTF("8dbm\n");
    break;
    case 0x05:
    nrf_gpio_pin_set(RADIO_ATT_1dbm);
    nrf_gpio_pin_set(RADIO_ATT_2dbm);
    nrf_gpio_pin_set(RADIO_ATT_4dbm);
    nrf_gpio_pin_set(RADIO_ATT_8dbm);
    nrf_gpio_pin_clear(RADIO_ATT_16dbm);
    curr_attenuator = RADIO_16dBm;
    PRINTF("16dbm\n");
    break;
    case 0x06:
    nrf_gpio_pin_clear(RADIO_ATT_1dbm);
    nrf_gpio_pin_clear(RADIO_ATT_2dbm);
    nrf_gpio_pin_clear(RADIO_ATT_4dbm);
    nrf_gpio_pin_clear(RADIO_ATT_8dbm);
    nrf_gpio_pin_clear(RADIO_ATT_16dbm);
    curr_attenuator = RADIO_31dBm;
    PRINTF("31dbm\n");
    break;
    default:
    nrf_gpio_pin_set(RADIO_ATT_1dbm);
    nrf_gpio_pin_set(RADIO_ATT_2dbm);
    nrf_gpio_pin_set(RADIO_ATT_4dbm);
    nrf_gpio_pin_set(RADIO_ATT_8dbm);
    nrf_gpio_pin_set(RADIO_ATT_16dbm);
    curr_attenuator = RADIO_0dBm;
    PRINTF("0dbm\n");
    break;
  }
}
/*---------------------------------------------------------------------------*/
static void
app_terminating(void)
{
  nrf_gpio_pin_set(RADIO_PIN_0);
  nrf_gpio_pin_set(RADIO_PIN_1);
  nrf_gpio_pin_set(RADIO_PIN_2);
  nrf_gpio_pin_set(RADIO_PIN_3);
  nrf_gpio_pin_set(RADIO_PIN_4);
}
/*---------------------------------------------------------------------------*/
static struct app_lifecycle_event lifecycle_event = {
	.name = "hw_bsp_rfatte",
	.terminating = app_terminating
};
/*---------------------------------------------------------------------------*/
int
bsp_rfatte_write(const void *buf, uint32_t len, uint32_t offset)
{
  uint8_t *p_attr = (uint8_t*)buf;
  if (p_attr[0] >= RADIO_0dBm && p_attr[0] <= RADIO_31dBm) {
    set_attenuator(p_attr[0]);
    return ENONE;
  }
  return EINVAL;
}
/*---------------------------------------------------------------------------*/
int
bsp_rfatte_read(void *buf, uint32_t len, uint32_t offset)
{
  uint8_t *p_attr = (uint8_t*)buf;
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
bsp_rfatte_init(void)
{
	app_lifecycle_register(&lifecycle_event);
  nrf_gpio_cfg_output(RADIO_PIN_0);
  nrf_gpio_cfg_output(RADIO_PIN_1);
  nrf_gpio_cfg_output(RADIO_PIN_2);
  nrf_gpio_cfg_output(RADIO_PIN_3);
  nrf_gpio_cfg_output(RADIO_PIN_4);
  set_attenuator(RADIO_0dBm);
}
/*---------------------------------------------------------------------------*/
