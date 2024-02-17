#ifndef MLZ_FILE_H
#define MLZ_FILE_H

#define uint64 unsigned long long int
#define uchar unsigned char


uchar *fLoad(const char *filename, volatile uint64 *size);
char fSave(const char *filename, const uchar *data, uint64 size);

#endif //MLZ_FILE_H
