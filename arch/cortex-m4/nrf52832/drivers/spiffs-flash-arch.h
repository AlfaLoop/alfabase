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
#ifndef _SPIFFS_FLASH_ARCH_H
#define _SPIFFS_FLASH_ARCH_H

#include <stdbool.h>
#include <stdint.h>
#include "spiffs/spiffs.h"
#include "bsp_init.h"


// W25Q80 chip
// 8M-bit/1024K-bytes
// 256-byte per programmable page (Program one to 256 bytes < 1ms)
// Uniform 4KB Sectors, 32KB & 64KB Blocks
// 1mA active current, <1uA Power-down
#define W25Q80_SIZE 				   1024*1024
#define W25Q80_PAGES 				   4096
#define W25Q80_PAGE_SIZE 			  256
#define W25Q80_SECTOR_SIZE 			4096

// W25Q40 chip
// 4M-bit/512K-bytes
// 256-byte per programmable page (Program one to 256 bytes < 1ms)
// Uniform 4KB Sectors, 32KB & 64KB Blocks
// 1mA active current, <1uA Power-down
#define W25Q40_SIZE 				   512*1024
#define W25Q40_PAGES 				     2048
#define W25Q40_PAGE_SIZE 			    256
#define W25Q40_SECTOR_SIZE 			  4096

// W25Q20 chip
// 2M-bit/256K-bytes
// 256-byte per programmable page (Program one to 256 bytes < 1ms)
// Uniform 4KB Sectors, 32KB & 64KB Blocks
// 1mA active current, <1uA Power-down
#define W25Q20_SIZE 				   256*1024
#define W25Q20_PAGES 				     1024
#define W25Q20_PAGE_SIZE 			    256
#define W25Q20_SECTOR_SIZE 			  4096

extern spiffs extfs;
int spiffs_flash_arch_init(uint8_t format);

#endif /* _SPIFFS_FLASH_ARCH_H */
