#include "kerl.h"
#include "conversion.h"
#include "sha3.h"

// Keccak 384 object
static SHA3_CTX ctx;
static unsigned char bytes_out[48] = {0};


int kerl_initialize(void)
{
    keccak_384_Init(&ctx);
    return 0;
}

int kerl_absorb_bytes(const unsigned char bytes_in[], uint16_t len)
{
    keccak_Update(&ctx, bytes_in, len);
    return 0;
}

int kerl_absorb_trits(const trit_t trits_in[], uint16_t len)
{
    for (uint8_t i = 0; i < (len/243); i++) {
        // Last trit to zero
        trit_t trits[243] = {0};
        memcpy(trits, &trits_in[i*243], 242);

        // First, convert to bytes
        int32_t words[12];
        unsigned char bytes[48];
        trits_to_words(trits, words);
        words_to_bytes(words, bytes, 12);
        keccak_Update(&ctx, bytes, 48);
    }
    return 0;
}

int kerl_squeeze_trits(trit_t trits_out[], uint16_t len)
{
    (void) len;
    unsigned char bytes_out[48] = {0};
    keccak_Final(&ctx, bytes_out);

    // Convert to trits
    int32_t words[12];
    bytes_to_words(bytes_out, words, 12);
    words_to_trits(words, trits_out);

    // Last trit zero
    trits_out[242] = 0;

    // Flip bytes
    for (uint8_t i = 0; i < 48; i++) {
        bytes_out[i] = bytes_out[i] ^ 0xFF;
    }

    keccak_384_Init(&ctx);
    keccak_Update(&ctx, bytes_out, 48);

    return 0;
}


//utilize encoded format
int kerl_absorb_trints(trint_t *trints_in, uint16_t len)
{
    for (uint8_t i = 0; i < (len/49); i++) {
        // First, convert to bytes
        int32_t words[12];
        unsigned char bytes[48];
        //Convert straight from trints to words
        trints_to_words(trints_in, words);
        words_to_bytes(words, bytes, 12);
        kerl_absorb_bytes(bytes, 48);
    }
    return 0;
}

//utilize encoded format
int kerl_squeeze_trints(trint_t *trints_out, uint16_t len) {
    (void) len;

    // Convert to trits
    int32_t words[12];
    bytes_to_words(bytes_out, words, 12);
    words_to_trints(words, &trints_out[0]);


    //-- Setting last trit to 0
    trit_t trits[3];
    //grab and store last clump of 3 trits
    trint_to_trits(trints_out[48], &trits[0], 3);
    trits[2] = 0; //set last trit to 0
    //convert new trit set back to trint and store
    trints_out[48] = trits_to_trint(&trits[0], 3);

    // TODO: Check if the following is needed. Seems to do nothing.

    // Flip bytes
    for (uint8_t i = 0; i < 48; i++) {
        bytes_out[i] = bytes_out[i] ^ 0xFF;
    }

    kerl_initialize();
    kerl_absorb_bytes(bytes_out,48);

    return 0;
}
