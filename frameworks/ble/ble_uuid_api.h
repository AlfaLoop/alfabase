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
#ifndef _BLE_UUID_API_H
#define _BLE_UUID_API_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

enum {
  BLE_UUID_TYPE_16 = 16,
  BLE_UUID_TYPE_128 = 128,
};

typedef struct {
  uint8_t type;
  uint16_t value;
} BleUuid16;

typedef struct {
  uint8_t type;
  uint8_t value[16];
} BleUuid128;

#define BLE_UUID16_INIT(uuid16)         \
    {                                   \
      .type = BLE_UUID_TYPE_16,        \
      .value = (uuid16),             \
    }

#define BLE_UUID128_INIT(uuid128...)    \
    {                                   \
      .type = BLE_UUID_TYPE_128,        \
      .value = { uuid128 },             \
    }

#ifdef __cplusplus
}
#endif
#endif /* _BLE_UUID_API_H */
