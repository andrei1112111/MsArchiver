#ifndef MLZ_FILE_H
#define MLZ_FILE_H

#define uint64 unsigned long long int
#define uchar unsigned char

char isFile(const char *filename);
char fCheck(const char *filename);
char exCheck(const char *filename);

void fGetContent(char **filenames, uint64 fCount);
void fArcData(char **filenames, uint64 fCount);
void fDArkData(char **filenames, uint64 fCount);

#endif //MLZ_FILE_H
