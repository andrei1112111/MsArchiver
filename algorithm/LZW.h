#ifndef MLZ_LZW_H
#define MLZ_LZW_H

#define uint64 unsigned long long int
#define uchar unsigned char

uint64 *lzwEncode(const uchar *input, uint64 size_inp, uint64 *result_len, uchar *uSize, uint64 max_dict_size, uint64 start_dict_size);
void lzwDecode(FILE *input, int uInpSize, uint64 size_inp, FILE *output);

#endif //MLZ_LZW_H
