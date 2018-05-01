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
#ifndef _OSLOCALTIME_API_H
#define _OSLOCALTIME_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

/* Framework API */
typedef struct{
  uint32_t (*getSeconds)(void);
  uint32_t (*getMinutes)(void);
  uint32_t (*getHour24)(void);
  uint32_t (*getHour12)(void);
  uint32_t (*getDayOfWeek)(void);
  uint32_t (*getDayOfMonth)(void);
  uint32_t (*getDayOfYear)(void);
  uint32_t (*getMonth)(void);
  uint32_t (*getYear)(void);
} Localtime;

// Singleton instance of HwInfo
Localtime* OSLocaltime(void);
/* Framework API */

/* Back-end */
void oslocaltime_api_init(void);
/* Back-end */


#ifdef __cplusplus
}
#endif
#endif /* _OSLOCALTIME_API_H */
