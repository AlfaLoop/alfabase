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
#ifndef _BOOTLOADER_H
#define _BOOTLOADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

uint32_t bootloader_start(void);

#ifdef __cplusplus
}
#endif

#endif /* _BOOTLOADER_H */
