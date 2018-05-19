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
int
bsp_rfatte_open(void *args)
{
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
bsp_rfatte_write(const void *buf, uint32_t len, uint32_t offset)
{
  return ENOSUPPORT;
}
/*---------------------------------------------------------------------------*/
int
bsp_rfatte_read(void *buf, uint32_t len, uint32_t offset)
{
  switch (offset) {
    case 0:
    {
    }
    break;
    case 1:
    {
    }
    break;
  }
  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
bsp_rfatte_close(void *args)
{
  return ENONE;
}
/*---------------------------------------------------------------------------*/
static void
app_terminating(void)
{
}
/*---------------------------------------------------------------------------*/
static struct app_lifecycle_event lifecycle_event = {
	.name = "hw_bsp_rfatte",
	.terminating = app_terminating
};
/*---------------------------------------------------------------------------*/
int
bsp_rfatte_init(void)
{
	app_lifecycle_register(&lifecycle_event);
}
/*---------------------------------------------------------------------------*/
