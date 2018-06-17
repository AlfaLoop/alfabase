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
#ifndef _NEST_PACKER_H_
#define _NEST_PACKER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "nest.h"

typedef struct {
	uint8_t len;
	uint8_t opcode;
	uint8_t seq;
	uint8_t version;
} nest_proto_header_t;

bool nest_packer_unpack(uint8_t *src, uint8_t *dst, nest_proto_header_t *header);
void nest_packer_pack(uint8_t *dst, uint8_t len, uint8_t *p_data, uint8_t opcode);

#ifdef __cplusplus
}
#endif
#endif /* _NEST_PACKER_H_ */
