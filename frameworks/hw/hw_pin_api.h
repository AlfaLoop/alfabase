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
#ifndef _HW_PIN_API_H
#define _HW_PIN_API_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Framework API */
enum {
  PIN_DEFAULT = 0x00,
  PIN_OUTPUT = 0x01,
  PIN_INPUT = 0x02
};

enum {
  PIN_OUTPUT_LOW = 0x00,
  PIN_OUTPUT_HIGH = 0x01,
  PIN_OUTPUT_TOGGLE
};

enum {
  PIN_INPUT_FLOATING = 0x00,
  PIN_INPUT_PULLUP = 0x01,
  PIN_INPUT_PULLDOWN = 0x02
};

enum {
  PIN_EDGE_NONE = 0x00,
  PIN_EDGE_RISING = 0x01,
  PIN_EDGE_FALLING = 0x02
};

typedef void (* PinEventHandler)(uint32_t pin, uint32_t edge);

typedef struct {
  int (* setup)(uint32_t pin, uint8_t value);
  int (* output)(uint32_t pin, uint8_t value);
  int (* input)(uint32_t pin, uint8_t value);
  int (* read)(uint32_t pin);
  int (* attachInterrupt)(uint32_t pin, uint32_t edge, PinEventHandler handler);
  int (* detachInterrupt)(void);
} Pin;
/* Framework API */

/* Back-end */
Pin* bsp_hw_pin_api_retrieve();
void hw_pin_terminating(void);
/* Back-end */

#ifdef __cplusplus
}
#endif
#endif /* _HW_PIN_API_H */
