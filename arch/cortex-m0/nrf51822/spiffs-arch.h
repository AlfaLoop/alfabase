/* Copyright (c) 2016, Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *   * Neither the name of Nordic Semiconductor ASA nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
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

#define CEIL_DIV(A, B)      \
    (((A) + (B) - 1) / (B))


/* Minimum erasable unit. */
#define FLASH_PAGE_SIZE           1024
#define LOG_FLASH_PAGE_SIZE       128       // 16 pages in a block  4096 / 16 = 256
/* Last 20 pages reserved for boot loader. */
//#define FLASH_PAGES               16

#define NRF_FLASH_MAP_SD_START		      	0x00000
#define NRF_FLASH_MAP_SD_SIZE		          0x1B000
#define NRF_FLASH_MAP_KERNEK_START      	0x1B000
#define NRF_FLASH_MAP_KERNEK_SIZE      		0x22400
#define NRF_FLASH_MAP_FS_START		      	0x3E400
#define NRF_FLASH_MAP_FS_SIZE		          0X01800


extern spiffs nrf_spiffs;
#define SYSFS nrf_spiffs

void nrf_spiffs_flash_read(uint32_t addr, uint8_t *data, uint32_t size);
void nrf_spiffs_flash_write(uint32_t addr, uint8_t *data, uint32_t size);
void nrf_spiffs_flash_erase(uint32_t addr, uint32_t size);
void nrf_flash_erase_and_write(uint32_t address, const void *data, uint32_t length);
uint8_t is_fs_arch_processing(void);
int nrf_spiffs_arch_init(uint8_t format);

#endif /* _SPIFFS_ARCH_H */
