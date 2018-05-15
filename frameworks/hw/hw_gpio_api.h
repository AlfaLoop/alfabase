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
#ifndef _HW_GPIO_API_H
#define _HW_GPIO_API_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Framework API */
enum {
  GPIO_DEFAULT = 0x00,
  GPIO_OUTPUT = 0x01,
  GPIO_INPUT = 0x02
};

enum {
  GPIO_OUTPUT_LOW = 0x00,
  GPIO_OUTPUT_HIGH = 0x01,
  GPIO_OUTPUT_TOGGLE
};

enum {
  GPIO_INPUT_FLOATING = 0x00,
  GPIO_INPUT_PULLUP = 0x01,
  GPIO_INPUT_PULLDOWN = 0x02
};

enum {
  GPIO_EDGE_NONE = 0x00,
  GPIO_EDGE_RISING = 0x01,
  GPIO_EDGE_FALLING = 0x02
};

typedef void (* GpioEventHandler)(uint32_t pin, uint32_t edge);

typedef struct {
  int (* setup)(uint32_t pin, uint8_t value);
  int (* output)(uint32_t pin, uint8_t value);
  int (* input)(uint32_t pin, uint8_t value);
  int (* read)(uint32_t pin);
  int (* attachInterrupt)(uint32_t pin, uint32_t edge, GpioEventHandler handler);
  int (* detachInterrupt)(void);
} Gpio;
/* Framework API */

/* Back-end */
Gpio* bsp_hw_gpio_api_retrieve();
void hw_gpio_terminating(void);
/* Back-end */

#ifdef __cplusplus
}
#endif
#endif /* _HW_GPIO_API_H */
