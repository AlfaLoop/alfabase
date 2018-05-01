/**
 *  Copyright (c) AlfaLoop, Inc. - All Rights Reserved
 *
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential.
 *
 *  Attribution — You must give appropriate credit, provide a link to the license, and
 *  indicate if changes were made. You may do so in any reasonable manner, but not in any
 *  way that suggests the licensor endorses you or your use.
 *
 *  NonCommercial — You may not use the material for commercial purposes under unauthorized.
 *
 *  NoDerivatives — If you remix, transform, or build upon the material, you may not
 *  distribute the modified material.
 *
 *  Written by Chun-Ting Ding <chunting.d@alfaloop.com> 2015-2016.
 *
 */
#ifndef _CRYPTO_H__
#define _CRYPTO_H__

#include "contiki.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef CRYPTO_PLATFORM_ARCH
#include "crypto-arch.h"
#define CRYPTO 		CRYPTO_PLATFORM_ARCH
#else
#define CRYPTO 		nrf_xxtea_driver			// default
#endif

/**
 * Structure of crypto drivers.
 */
struct crypto_driver {
   /**
   * \brief init.
   */
  void (* init)(void);

  /**
   * \brief Sets the current key.
   */
  void (* set_key)(uint8_t *key);

  /**
   * \brief Encrypt
   */
  bool (* encrypt)(uint8_t * dst, const uint8_t * src, uint32_t len);

   /**
   * \brief Decrypt
   */
  bool (* decrypt)(uint8_t * dst, const uint8_t * src, uint32_t len);
};

extern const struct crypto_driver CRYPTO;

#endif // _CRYPT_H__
