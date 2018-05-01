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
