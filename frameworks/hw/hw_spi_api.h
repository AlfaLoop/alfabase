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
#ifndef _HW_SPI_API_H
#define _HW_SPI_API_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Framework API */

typedef struct {
  int (* init)(uint32_t mosi_pin, uint32_t miso_pin, uint32_t sclk_pin, uint32_t speed);
  int (* write)(uint32_t address, uint8_t *value);
  int (* read)(uint32_t address, uint8_t *value);
  int (* close)(void);
} Spi;
/* Framework API */

/* Back-end */
Spi* bsp_hw_spi_api_retrieve(uint8_t number);
void hw_spi_terminating(void);
/* Back-end */

#ifdef __cplusplus
}
#endif
#endif /* _HW_SPI_API_H */
