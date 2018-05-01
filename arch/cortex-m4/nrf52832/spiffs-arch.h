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
#ifndef _SPIFFS_ARCH_H
#define _SPIFFS_ARCH_H

#include <stdbool.h>
#include <stdint.h>
#include "spiffs/spiffs.h"
#include "bsp_init.h"

#define CEIL_DIV(A, B)      \
    (((A) + (B) - 1) / (B))


/* Minimum erasable unit. */
#define FLASH_PAGE_SIZE           4096
#define LOG_FLASH_PAGE_SIZE       256       // 16 pages in a block  4096 / 16 = 256
/* Last 20 pages reserved for boot loader. */
//#define FLASH_PAGES               128

#define NRF_FLASH_MAP_SD_START		      0x00000
#define NRF_FLASH_MAP_SD_SIZE		        0x1F000      // 124 KB
#define NRF_FLASH_MAP_KERNEK_START      0x1F000
#define NRF_FLASH_MAP_KERNEK_SIZE       0x49000      // 292 KB
#define NRF_FLASH_MAP_FS_START		      0x68000
#define NRF_FLASH_MAP_FS_SIZE		        0x10000      // 64 KB
#define NRF_FLASH_MAP_BOOTLADER_START		0x78000
#define NRF_FLASH_MAP_BOOTLADER_SIZE		0x7000      // 32 KB

extern spiffs nrf_spiffs;
#define SYSFS nrf_spiffs

void nrf_spiffs_flash_read(uint32_t addr, uint8_t *data, uint32_t size);
void nrf_spiffs_flash_write(uint32_t addr, uint8_t *data, uint32_t size);
void nrf_spiffs_flash_erase(uint32_t addr, uint32_t size);
void nrf_flash_erase_and_write(uint32_t address, const void *data, uint32_t length);
uint8_t is_fs_arch_processing(void);
int nrf_spiffs_arch_init(uint8_t format);

#endif /* _SPIFFS_ARCH_H */
