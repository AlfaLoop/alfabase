/**
 *  Copyright (c) 2016 AlfaLoop Technology Co., Ltd. All Rights Reserved.
 *
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential.
 *
 *  Attribution - You must give appropriate credit, provide a link to the license, and
 *  indicate if changes were made. You may do so in any reasonable manner, but not in any
 *  way that suggests the licensor endorses you or your use.
 *
 *  NonCommercial - You may not use the material for commercial purposes under unauthorized.
 *
 *  NoDerivatives - If you remix, transform, or build upon the material, you may not
 *  distribute the modified material.
 */
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
