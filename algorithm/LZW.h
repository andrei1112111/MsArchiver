#ifndef MLZA_LZW_H
#define MLZA_LZW_H

unsigned long long int *lzw_encode(const unsigned char *input, unsigned long long int size_inp, unsigned long long int *result_len);

unsigned char *lzw_decode(const unsigned long long int *input, unsigned long long int size_inp, unsigned long long int *result_len);

#endif //MLZA_LZW_H
