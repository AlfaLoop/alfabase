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
#ifndef _HARDFAULT_H_
#define _HARDFAULT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "contiki.h"

typedef enum {
  HADRFAULT_NONE_DEFINE = 0x00000000,
  HADRFAULT_DIV_0,
  HADRFAULT_UNALIGNED_ACCESS,
  HADRFAULT_NO_COPROCESSOR,
  HADRFAULT_INVALID_PC_LOAD,
  HADRFAULT_INVALID_STATE,
  HADRFAULT_UNDEFINED_INSTRUCTION,
  HADRFAULT_BUS,
  HADRFAULT_MEMORT_MANAGEMENT
} hardfault_type_t;

typedef struct {
  uint32_t type;
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r12;
  uint32_t lr;
  uint32_t pc;
  uint32_t psr;
} hardfault_dump_t;

PROCESS_NAME(hardfault_process);

#ifdef __cplusplus
}
#endif
#endif /* _HARDFAULT_H_ */
