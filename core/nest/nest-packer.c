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
#include "contiki.h"
#include "nest-packer.h"
#include "libs/util/xor.h"
/*---------------------------------------------------------------------------*/
#if defined(DEBUG_ENABLE)
#define DEBUG_MODULE 1
#if DEBUG_MODULE
#include "dev/syslog.h"
#define PRINTF(...) syslog(__VA_ARGS__)
#else
#define PRINTF(...)
#endif  /* DEBUG_MODULE */
#else
#define PRINTF(...)
#endif  /* DEBUG_ENABLE */
/*---------------------------------------------------------------------------*/
static uint8_t m_seq = 0;
/*---------------------------------------------------------------------------*/
bool
nest_packer_unpack(uint8_t *src, uint8_t *dst, nest_proto_header_t *header)
{
	uint8_t version, src_xor;
	// 60 26 6c 04 5a e4 06 d4 00 00 00 00 00 00 00 00 00 00 00 00

	header->version = (src[0] & 0xE0) >> 5 ;
  header->seq = (src[0] & 0x1F) ;
	header->opcode = src[1];
  header->len = src[3];

  // version 3
	if (header->version != NEST_PROTOCOL_VERSION) {
		PRINTF("[nest packer] version not match\n");
		return false;
	}

  memcpy(dst, &src[4], 16);

#if DEBUG_MODULE > 1
  PRINTF("[nest packer] opcode %d length %d seq %d\n", header->opcode, header->len, header->seq);
	PRINTF("[nest packer] unpack ");
	for (uint8_t i = 0; i < 16; i++) {
		PRINTF("%2x ", dst[i]);
	}
	PRINTF("\n");
#endif

	src_xor = calculate_xor(dst, header->len);
	if (src_xor != src[2]) {
		PRINTF("[nest packer] xor not match\n");
		return false;
	}
	return true;
}
/*---------------------------------------------------------------------------*/
void
nest_packer_pack(uint8_t *dst, uint8_t len, uint8_t *p_data, uint8_t opcode)
{
	uint8_t idx;

	dst[0] = NEST_PROTOCOL_VERSION << 5 | (m_seq & 0x1F);
	dst[1] = opcode;
	dst[2] = calculate_xor(p_data, len);
  dst[3] = len;

	memset(&dst[4], 0x00, 16);
	memcpy(&dst[4], p_data, len);

#if DEBUG_MODULE > 1
  PRINTF("[nest packer] opcode 0x%02x length %d\n", dst[1], len);
	PRINTF("[nest packer] pack ");
	for (uint8_t i = 0; i < 20; i++) {
		PRINTF("%2x ", dst[i]);
	}
	PRINTF("\n");
#endif

  m_seq++;
	return;
}
/*---------------------------------------------------------------------------*/
