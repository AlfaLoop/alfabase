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
#ifndef SYMTAB_H_
#define SYMTAB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "loader/symbols.h"

void *symtab_lookup(const char *name);

void symtab_init(void);
void symtab_add(struct symbols *symb);

#ifdef __cplusplus
}
#endif

#endif /* SYMTAB_H_ */
