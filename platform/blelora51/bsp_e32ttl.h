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
#ifndef ___BSP_E32TTL_H
#define ___BSP_E32TTL_H
#ifdef __cplusplus
extern "C" {
#endif

#include "contiki.h"
#include "frameworks/hw/hw_api.h"


int bsp_e32ttl_init(void);
int bsp_e32ttl_open(void *args);
int bsp_e32ttl_write(const void *buf, uint32_t len, uint32_t offset);
int bsp_e32ttl_read(void *buf, uint32_t len, uint32_t offset);
int bsp_e32ttl_subscribe(void *buf, uint32_t len, HWCallbackHandler handler);
int bsp_e32ttl_close(void *args);

#ifdef __cplusplus
}
#endif
#endif /* ___BSP_E32TTL_H */
