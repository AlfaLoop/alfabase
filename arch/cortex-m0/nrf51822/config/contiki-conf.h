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
#ifndef _CONTIKI_CONF_H_
#define _CONTIKI_CONF_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "bsp_init.h"
#include "nrf.h"
#include "app_util.h"

#if !defined(CONTIKI_ELFLOADER_DATAMEMORY_SIZE_CONF)
  #error Non-defined CONTIKI_ELFLOADER_DATAMEMORY_SIZE_CONF in bsp_init.h
#endif

#if !defined(CONTIKI_ELFLOADER_TEXTMEMORY_SIZE_CONF)
  #error Non-defined CONTIKI_ELFLOADER_TEXTMEMORY_SIZE_CONF in bsp_init.h
#endif

// Compiler configurations.
// CCIF and CLIF are defined but used only in Windows based platforms
#define CCIF
#define CLIF

/* These names are deprecated, use C99 names. */
typedef uint8_t u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef int8_t s8_t;
typedef int16_t s16_t;
typedef int32_t s32_t;

// Platform typedefs are all unsigned 32 bit
typedef uint32_t clock_time_t;
typedef uint32_t hrtimer_clock_t;

// Contiki Frequency of main clock
#define CLOCK_CONF_SECOND 1024
/* Check if CLOCK_CONF_SECOND is power of 2 and less than or equal to 32.768 kHz */
#if (!(!(CLOCK_CONF_SECOND & (CLOCK_CONF_SECOND-1)) && CLOCK_CONF_SECOND && (CLOCK_CONF_SECOND<=32768)))
#error CLOCK_CONF_SECOND must be a power of 2 with a maximum frequency of 32768 Hz
#endif

// High resolution timer configure
#define HRTIMER_ARCH_SECOND 		1000000UL
#define HRTIMER_ARCH_CLOCK_LT(a,b)  ((int32_t)((a)-(b)) < 0)
#define HRTIMER_ARCH_CHANNEL   		1

#define HRTIMER_CONF_MAX_NUM	  	3
#define HRTIMER_CONF_DRIFT_FIX		8
#define HRTIMER_CONF_BITWITH 		0xFFFF

// Contiki process Loader configuration
#define ELFLOADER_CONF_TEXT_IN_ROM 0


#ifndef ELFLOADER_DATAMEMORY_SIZE
#ifdef CONTIKI_ELFLOADER_DATAMEMORY_SIZE_CONF
#define ELFLOADER_DATAMEMORY_SIZE CONTIKI_ELFLOADER_DATAMEMORY_SIZE_CONF
#else
#define ELFLOADER_DATAMEMORY_SIZE 0x100
#endif
#endif /* ELFLOADER_DATAMEMORY_SIZE */

#ifndef ELFLOADER_TEXTMEMORY_SIZE
#ifdef CONTIKI_ELFLOADER_TEXTMEMORY_SIZE_CONF
#define ELFLOADER_TEXTMEMORY_SIZE CONTIKI_ELFLOADER_TEXTMEMORY_SIZE_CONF
#else
#define ELFLOADER_TEXTMEMORY_SIZE 0x100
#endif
#endif /* ELFLOADER_TEXTMEMORY_SIZE */


// Contiki TICKLESS  mode
#define TICKLESS 				true

#ifdef __cplusplus
}
#endif
#endif /* _CONTIKI_CONF_H_ */
