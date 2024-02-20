#ifndef MLZ_LZW_H
#define MLZ_LZW_H

#define uint64 unsigned long long int
#define uchar unsigned char

uint64 *lzwEncode(const uchar *input, uint64 size_inp, volatile uint64 *result_len);
uchar *lzwDecode(const uint64 *input, uint64 size_inp, volatile uint64 *result_len);

#endif //MLZ_LZW_H
