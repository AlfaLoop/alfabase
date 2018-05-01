* Supported algorithm


#define CRYPTO_HASH_KEY_LENGTH    22
#define CRYPTO_SECRET_KEY_LENGTH  16


static uint8_t secret_key[16] = {0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92};
static uint8_t secret_salt[4] = {0x21, 0xA5, 0x5A, 0x2c};
static uint8_t secret_connection_key[16] = {0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x00, 0x00, 0x00, 0x00};
static uint8_t m_plain[26];
static counter_data_t m_counter_data;
static hmac_sha256_t m_hmac_sha256;
static uint8_t m_digest_buf[CRYPTO_SHA256_DIGEST_SIZE];
static uint8_t m_raw_key[CRYPTO_HASH_KEY_LENGTH] = {
  0x00, 0x00, 0x00, 0x00, 0x00,  				   // LINE Simple Beacon HWID
  0x00, 0x00, 0x00, 0x00,						   // Device Id
  0x00, 									       // Battery Level
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Counter field
  0x21, 0xA5, 0x5A, 0x2c													 // Salt
};



// set up the secure key
hmac_sha256_setkey(&m_hmac_sha256, secret_key, CRYPTO_SECRET_KEY_LENGTH);
hmac_sha256_init(&m_hmac_sha256);
hmac_sha256_update(&m_hmac_sha256, m_raw_key, CRYPTO_HASH_KEY_LENGTH);

// Get the final digest
hmac_sha256_final(&m_hmac_sha256, m_digest_buf);

uint8_t resu_tmp[4];
for(int i = 0; i < 4; i++) {
	for(int j = 0; j < 8; j++) {
		resu_tmp[i] ^= m_digest_buf[i+(j*4)];
	}
}
 