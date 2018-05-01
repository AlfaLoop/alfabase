#ifndef CONVERSION_H
#define CONVERSION_H

#include "iota_types.h"

#define TYPE_INT 1
#define TYPE_UINT 2
#define TYPE_STR 3
#define TOP 1
#define BOT 2

static const trit_t trits_mapping[27][3] =
       {{-1, -1, -1}, { 0, -1, -1}, {1, -1, -1},
        {-1,  0, -1}, { 0,  0, -1}, {1,  0, -1},
        {-1,  1, -1}, { 0,  1, -1}, {1,  1, -1},
        {-1, -1,  0}, { 0, -1,  0}, {1, -1,  0},
        {-1,  0,  0}, { 0,  0,  0}, {1,  0,  0},
        {-1,  1,  0}, { 0,  1,  0}, {1,  1,  0},
        {-1, -1,  1}, { 0, -1,  1}, {1, -1,  1},
        {-1,  0,  1}, { 0,  0,  1}, {1,  0,  1},
        {-1,  1,  1}, { 0,  1,  1}, {1,  1,  1}};

int trits_to_trytes(const trit_t trits_in[], tryte_t trytes_out[], uint32_t trit_len);
int trytes_to_trits(const tryte_t trytes_in[], trit_t trits_out[], uint32_t tryte_len);

int int32_to_trits(const int32_t value, trit_t trits_out[], uint8_t max_len);
int tryte_to_trits(const tryte_t tryte, trit_t trits_out[]);

int trits_to_words(const trit_t trits_in[], int32_t words_out[]);
int words_to_trits(const int32_t words_in[], trit_t trits_out[]);

int trints_to_words(trint_t *trints_in, int32_t words_out[]);
int words_to_trints(const int32_t words_in[], trint_t *trints_out);

int words_to_bytes(const int32_t words_in[], unsigned char bytes_out[], uint8_t word_len);
int bytes_to_words(const unsigned char bytes_in[], int32_t words_out[], uint8_t word_len);

int chars_to_trytes(const char chars_in[], tryte_t trytes_out[], uint8_t len);
int trytes_to_chars(const tryte_t trytes_in[], char chars_out[], uint16_t len);

void uint_to_str(uint32_t i, char *str, uint8_t len);
void int_to_str(int i, char *str, uint8_t len);
uint32_t str_to_int(char *str, uint8_t len);

void specific_243trits_to_49trints(int8_t *trits, int8_t *trints_r);
void specific_49trints_to_243trits(int8_t *trints, int8_t *trits_r);
void trint_to_trits(int8_t integ, int8_t *trits_r, int8_t sz);
int8_t trits_to_trint(int8_t *trits, int8_t sz);

#endif // CONVERSION_H
