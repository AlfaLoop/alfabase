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
#ifndef _OSFILE_API_H
#define _OSFILE_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "spiffs/spiffs.h"
#include "bsp_init.h"

 #if !defined(STORAGE_SYSC_FS_INSTANCE_CONF)
   #error Non-defined STORAGE_SYSC_FS_INSTANCE_CONF in bsp_init.h
 #endif

/* Framework API */
#define OSFILE_DATA_MAX_LENGTH 			64
#define OSFILE_DATA_MAX_OPEN 			  4

typedef enum {
  SeekSet = 0x00,
  SeekCur,
  SeekEnd
} SeekMode;

typedef struct {
  int (*open)(int16_t *fd, const uint32_t key, const char *mode);
  int (*write)(const int16_t fd, const void *data, const uint32_t size);
  int (*read)(const int16_t fd, void *data, const uint32_t size);
  int (*seek)(const int16_t fd, const uint32_t offset, const SeekMode mode);
  int (*close)(const int16_t fd);
  int (*size)(const uint32_t key);
  int (*remove)(const uint32_t key);
  int (*removeAll)(void);
  int (*space)(uint32_t *total, uint32_t *used);
} FileIO;

FileIO *OSFileIO(void);
/* Framework API */

/* Back-end */
#ifdef STORAGE_SYSC_FS_INSTANCE_CONF
#define SFS STORAGE_SYSC_FS_INSTANCE_CONF
#else
#define SFS SYSFS
#endif
void osfile_api_init(void);
/* Back-end */

#ifdef __cplusplus
}
#endif
#endif /* _OSFILE_API_H */
