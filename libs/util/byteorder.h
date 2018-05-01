/**
 *  Copyright (c) 2016 AlfaLoop Inc. All Rights Reserved.
 *
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential.
 *
 *  Attribution ¡X You must give appropriate credit, provide a link to the license, and
 *  indicate if changes were made. You may do so in any reasonable manner, but not in any 
 *  way that suggests the licensor endorses you or your use.
 *
 *  NonCommercial ¡X You may not use the material for commercial purposes under unauthorized.
 *
 *  NoDerivatives ¡X If you remix, transform, or build upon the material, you may not 
 *  distribute the modified material.
 */
#ifndef _BYTEORDER_H__
#define _BYTEORDER_H__

#include <stdint.h>
#include <stdbool.h>
#include "compiler_abstraction.h"

typedef uint8_t uint16_le_t[2];
typedef uint8_t uint32_le_t[4];


#define IS_POWER_OF_TWO(A) ( ((A) != 0) && ((((A) - 1) & (A)) == 0) )

static __INLINE uint8_t uint16_encode(uint16_t value, uint8_t * p_encoded_data)
{
    p_encoded_data[0] = (uint8_t) ((value & 0x00FF) >> 0);
    p_encoded_data[1] = (uint8_t) ((value & 0xFF00) >> 8);
    return sizeof(uint16_t);
}
    
static __INLINE uint8_t uint32_encode(uint32_t value, uint8_t * p_encoded_data)
{
    p_encoded_data[0] = (uint8_t) ((value & 0x000000FF) >> 0);
    p_encoded_data[1] = (uint8_t) ((value & 0x0000FF00) >> 8);
    p_encoded_data[2] = (uint8_t) ((value & 0x00FF0000) >> 16);
    p_encoded_data[3] = (uint8_t) ((value & 0xFF000000) >> 24);
    return sizeof(uint32_t);
}


static __INLINE uint8_t uint32_encode_le(uint32_t value, uint8_t * p_encoded_data)
{
    p_encoded_data[3] = (uint8_t) ((value & 0x000000FF) >> 0);
    p_encoded_data[2] = (uint8_t) ((value & 0x0000FF00) >> 8);
    p_encoded_data[1] = (uint8_t) ((value & 0x00FF0000) >> 16);
    p_encoded_data[0] = (uint8_t) ((value & 0xFF000000) >> 24);
    return sizeof(uint32_t);
}


static __INLINE uint16_t uint16_decode(const uint8_t * p_encoded_data)
{
        return ( (((uint16_t)((uint8_t *)p_encoded_data)[0])) | 
                 (((uint16_t)((uint8_t *)p_encoded_data)[1]) << 8 ));
}

static __INLINE uint32_t uint32_decode(const uint8_t * p_encoded_data)
{
    return ( (((uint32_t)((uint8_t *)p_encoded_data)[0]) << 0)  |
             (((uint32_t)((uint8_t *)p_encoded_data)[1]) << 8)  |
             (((uint32_t)((uint8_t *)p_encoded_data)[2]) << 16) |
             (((uint32_t)((uint8_t *)p_encoded_data)[3]) << 24 ));
}


static __INLINE uint32_t uint32_decode_le(const uint8_t * p_encoded_data)
{
    return ( (((uint32_t)((uint8_t *)p_encoded_data)[0]) << 24)  |
             (((uint32_t)((uint8_t *)p_encoded_data)[1]) << 16)  |
             (((uint32_t)((uint8_t *)p_encoded_data)[2]) << 8) |
             (((uint32_t)((uint8_t *)p_encoded_data)[3]) << 0 ));
}

static __INLINE bool is_word_aligned(void * p)
{
    return (((uintptr_t)p & 0x03) == 0);
}

#endif // _BYTEORDER_H__
