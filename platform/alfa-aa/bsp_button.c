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
#include "bsp_button.h"
#include <stdint.h>
#include <stdbool.h>
#include "frameworks/hw/hw_api.h"
#include "frameworks/hw/hw_api_null.h"
#include "frameworks/app_eventpool.h"
#include "frameworks/app_lifecycle.h"
#include "sys/systime.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "gpiote.h"
#include "sys/timer.h"
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
static void
app_terminating(void)
{
}
/*---------------------------------------------------------------------------*/
static struct app_lifecycle_event lifecycle_event = {
	.name = "hw_bsp_button",
	.terminating = app_terminating
};
/*---------------------------------------------------------------------------*/
int
bsp_button_read(void *buf, uint32_t len, uint32_t offset)
{
  uint8_t *p_button_ctrl = (uint8_t*)buf;
  uint8_t b0 = nrf_gpio_pin_read(BUTTON0);
  uint8_t b1 = nrf_gpio_pin_read(BUTTON1);
  uint8_t b2 = nrf_gpio_pin_read(BUTTON2);
  uint8_t b3 = nrf_gpio_pin_read(BUTTON3);
  if (b0 == 0) {
    p_button_ctrl[0] = APP_POWER_MIN;
	} else if(b2 == 0) {
    p_button_ctrl[0] = APP_POWER_MAX;
	} else if(b3 == 0) {
    p_button_ctrl[0] = APP_POWER_MID;
	} else if(b1 == 0) {
    p_button_ctrl[0] = APP_POWER_CLOSE;
	}
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
bsp_button_subscribe(void *buf, uint32_t len, HWCallbackHandler handler)
{
  return ENOSUPPORT;
}
/*---------------------------------------------------------------------------*/
int
bsp_button_init(void)
{
  nrf_gpio_cfg_input(BUTTON0, NRF_GPIO_PIN_NOPULL);
  nrf_gpio_cfg_input(BUTTON1, NRF_GPIO_PIN_NOPULL);
  nrf_gpio_cfg_input(BUTTON2, NRF_GPIO_PIN_NOPULL);
  nrf_gpio_cfg_input(BUTTON3, NRF_GPIO_PIN_NOPULL);
	app_lifecycle_register(&lifecycle_event);
}
/*---------------------------------------------------------------------------*/
