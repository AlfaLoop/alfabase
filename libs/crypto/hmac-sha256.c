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
#include "hmac-sha256.h"
#include "contiki.h"
#include "errno.h"
/*---------------------------------------------------------------------------*/
#define Sigma0(a)(ROTR((a), 2) ^ ROTR((a), 13) ^ ROTR((a), 22))
#define Sigma1(a)(ROTR((a), 6) ^ ROTR((a), 11) ^ ROTR((a), 25))
#define sigma0(a)(ROTR((a), 7) ^ ROTR((a), 18) ^ ((a) >> 3))
#define sigma1(a)(ROTR((a), 17) ^ ROTR((a), 19) ^ ((a) >> 10))

#define Ch(a, b, c)(((a) & (b)) ^ ((~(a)) & (c)))
#define Maj(a, b, c)(((a) & (b)) ^ ((a) & (c)) ^ ((b) & (c)))


/*
 * Initializing SHA-256 Hash constant words K.
 * These values correspond to the first 32 bits of the fractional parts of the
 * cube roots of the first 64 primes between 2 and 311.
 */
static const uint32_t k256[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
	0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
	0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
	0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
	0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
	0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};


/*---------------------------------------------------------------------------*/
static inline
uint32_t ROTR(uint32_t a, uint32_t n)
{
	return (((a) >> n) | ((a) << (32 - n)));
}
/*---------------------------------------------------------------------------*/
static inline uint32_t BigEndian(const uint8_t **c)
{
	uint32_t n = 0;

	n = (((uint32_t)(*((*c)++))) << 24);
	n |= ((uint32_t)(*((*c)++)) << 16);
	n |= ((uint32_t)(*((*c)++)) << 8);
	n |= ((uint32_t)(*((*c)++)));
	return n;
}
/*---------------------------------------------------------------------------*/
static void
rekey(uint8_t *key, const uint8_t *new_key, uint32_t key_size)
{
	const uint8_t inner_pad = (uint8_t) 0x36;
	const uint8_t outer_pad = (uint8_t) 0x5c;
	uint32_t i;

	for (i = 0; i < key_size; ++i) {
		key[i] = inner_pad ^ new_key[i];
		key[i + CRYPTO_SHA256_BLOCK_SIZE] = outer_pad ^ new_key[i];
	}
	for (; i < CRYPTO_SHA256_BLOCK_SIZE; ++i) {
		key[i] = inner_pad; key[i + CRYPTO_SHA256_BLOCK_SIZE] = outer_pad;
	}
}
/*---------------------------------------------------------------------------*/
static void
transform(uint32_t *iv, const uint8_t *data)
{
  uint32_t a, b, c, d, e, f, g, h;
	uint32_t s0, s1;
	uint32_t t1, t2;
	uint32_t work_space[16];
	uint32_t n;
	uint32_t i;

	a = iv[0]; b = iv[1]; c = iv[2]; d = iv[3];
	e = iv[4]; f = iv[5]; g = iv[6]; h = iv[7];

	for (i = 0; i < 16; ++i) {
		n = BigEndian(&data);
		t1 = work_space[i] = n;
		t1 += h + Sigma1(e) + Ch(e, f, g) + k256[i];
		t2 = Sigma0(a) + Maj(a, b, c);
		h = g; g = f; f = e; e = d + t1;
		d = c; c = b; b = a; a = t1 + t2;
	}

	for ( ; i < 64; ++i) {
		s0 = work_space[(i+1)&0x0f];
		s0 = sigma0(s0);
		s1 = work_space[(i+14)&0x0f];
		s1 = sigma1(s1);

		t1 = work_space[i&0xf] += s0 + s1 + work_space[(i+9)&0xf];
		t1 += h + Sigma1(e) + Ch(e, f, g) + k256[i];
		t2 = Sigma0(a) + Maj(a, b, c);
		h = g; g = f; f = e; e = d + t1;
		d = c; c = b; b = a; a = t1 + t2;
	}

	iv[0] += a; iv[1] += b; iv[2] += c; iv[3] += d;
	iv[4] += e; iv[5] += f; iv[6] += g; iv[7] += h;
}
/*---------------------------------------------------------------------------*/
int
sha256_init(sha256_t *sha)
{
	/* input sanity check: */
	if (sha == NULL) {
		return ENULLP;
	}

	// Setting the initial state values.
	memset(sha, 0x00, sizeof(sha256_t));
	sha->iv[0] = 0x6a09e667;
	sha->iv[1] = 0xbb67ae85;
	sha->iv[2] = 0x3c6ef372;
	sha->iv[3] = 0xa54ff53a;
	sha->iv[4] = 0x510e527f;
	sha->iv[5] = 0x9b05688c;
	sha->iv[6] = 0x1f83d9ab;
	sha->iv[7] = 0x5be0cd19;

	return ENONE;
}
/*---------------------------------------------------------------------------*/
int
sha256_update(sha256_t *sha,  const uint8_t *data, uint32_t datalen)
{
  if (datalen == 0 || sha == NULL) {
		return EINVAL;
	}

  while (datalen-- > 0) {
		sha->leftover[sha->leftover_offset++] = *(data++);
		if (sha->leftover_offset >= CRYPTO_SHA256_BLOCK_SIZE) {
			transform(sha->iv, sha->leftover);
			sha->leftover_offset = 0;
			sha->bits_hashed += (CRYPTO_SHA256_BLOCK_SIZE << 3);
		}
	}
}
/*---------------------------------------------------------------------------*/
int
sha256_final(sha256_t *sha, uint8_t *digest)
{
  uint32_t i;

	/* input sanity check: */
	if (digest == NULL || sha == NULL) {
		return EINVAL;
	}

	sha->bits_hashed += (sha->leftover_offset << 3);

	sha->leftover[sha->leftover_offset++] = 0x80; /* always room for one byte */
	if (sha->leftover_offset > (sizeof(sha->leftover) - 8)) {
		/* there is not room for all the padding in this block */
		memset(sha->leftover + sha->leftover_offset, 0x00,
		     sizeof(sha->leftover) - sha->leftover_offset);
		transform(sha->iv, sha->leftover);
		sha->leftover_offset = 0;
	}

	/* add the padding and the length in big-Endian format */
	memset(sha->leftover + sha->leftover_offset, 0x00,
	     sizeof(sha->leftover) - 8 - sha->leftover_offset);
	sha->leftover[sizeof(sha->leftover) - 1] = (uint8_t)(sha->bits_hashed);
	sha->leftover[sizeof(sha->leftover) - 2] = (uint8_t)(sha->bits_hashed >> 8);
	sha->leftover[sizeof(sha->leftover) - 3] = (uint8_t)(sha->bits_hashed >> 16);
	sha->leftover[sizeof(sha->leftover) - 4] = (uint8_t)(sha->bits_hashed >> 24);
	sha->leftover[sizeof(sha->leftover) - 5] = (uint8_t)(sha->bits_hashed >> 32);
	sha->leftover[sizeof(sha->leftover) - 6] = (uint8_t)(sha->bits_hashed >> 40);
	sha->leftover[sizeof(sha->leftover) - 7] = (uint8_t)(sha->bits_hashed >> 48);
	sha->leftover[sizeof(sha->leftover) - 8] = (uint8_t)(sha->bits_hashed >> 56);

	/* hash the padding and length */
	transform(sha->iv, sha->leftover);

	/* copy the iv out to digest */
	for (i = 0; i < CRYPTO_SHA256_STATE_BLOCKS; ++i) {
		uint32_t t = *((uint32_t *) &sha->iv[i]);
		*digest++ = (uint8_t)(t >> 24);
		*digest++ = (uint8_t)(t >> 16);
		*digest++ = (uint8_t)(t >> 8);
		*digest++ = (uint8_t)(t);
	}

  /* destroy the current state */
  memset(sha, 0, sizeof(sha256_t));

  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
hmac_sha256_init(hmac_sha256_t *hmac, const uint8_t *key, uint32_t datalen)
{
	/* input sanity check: */
	if (hmac == NULL || key == NULL ||
	    datalen == 0) {
		return EINVAL;
	}

	const uint8_t dummy_key[CRYPTO_SHA256_BLOCK_SIZE];
  hmac_sha256_t dummy_state;

	if (datalen <= CRYPTO_SHA256_BLOCK_SIZE) {
		/*
		 * The next three lines consist of dummy calls just to avoid
		 * certain timing attacks. Without these dummy calls,
		 * adversaries would be able to learn whether the key_size is
		 * greater than CRYPTO_SHA256_BLOCK_SIZE by measuring the time
		 * consumed in this process.
		 */
		(void)sha256_init(&dummy_state.sha);
		(void)sha256_update(&dummy_state.sha,
				       dummy_key,
				       datalen);
		(void)sha256_final(&dummy_state.sha,
                      &dummy_state.key[CRYPTO_SHA256_DIGEST_SIZE]);

		/* Actual code for when key_size <= CRYPTO_SHA256_BLOCK_SIZE: */
		rekey(hmac->key, key, datalen);
	} else {
		(void)sha256_init(&hmac->sha);
		(void)sha256_update(&hmac->sha, key, datalen);
		(void)sha256_final(&hmac->sha, &hmac->key[CRYPTO_SHA256_DIGEST_SIZE]);
		rekey(hmac->key,
		      &hmac->key[CRYPTO_SHA256_DIGEST_SIZE],
		      CRYPTO_SHA256_DIGEST_SIZE);
	}

	(void)sha256_init(&hmac->sha);
	(void)sha256_update(&hmac->sha,
						 hmac->key,
						 CRYPTO_SHA256_BLOCK_SIZE);

	return ENONE;
}
/*---------------------------------------------------------------------------*/
int
hmac_sha256_update(hmac_sha256_t *hmac, const void *data, uint32_t datalen)
{
  /* input sanity check: */
	if (hmac == NULL) {
		return EINVAL;
	}

  (void)sha256_update(&hmac->sha, data, datalen);

  return ENONE;
}
/*---------------------------------------------------------------------------*/
int
hmac_sha256_final(hmac_sha256_t *hmac, uint8_t *data)
{
  /* input sanity check: */
	if (data == NULL || hmac == NULL) {
		return EINVAL;
	}

	(void)sha256_final(&hmac->sha, data);

	(void)sha256_init(&hmac->sha);
	(void)sha256_update(&hmac->sha,
			       &hmac->key[CRYPTO_SHA256_BLOCK_SIZE],
				CRYPTO_SHA256_BLOCK_SIZE);
	(void)sha256_update(&hmac->sha, data, CRYPTO_SHA256_DIGEST_SIZE);
	(void)sha256_final(&hmac->sha, data);

	/* destroy the current state */
	memset(hmac, 0, sizeof(hmac_sha256_t));

	return ENONE;
}
/*---------------------------------------------------------------------------*/
