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
