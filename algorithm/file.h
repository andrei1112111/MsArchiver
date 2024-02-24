#ifndef MLZ_FILE_H
#define MLZ_FILE_H

#define uint64 unsigned long long int
#define uchar unsigned char

char fCheck(const char *filename);
char exCheck(const char *filename);
char askUser(void);

void fGetContent(char **filenames, uint64 fCount, char skip);
void fArcData(char **filenames, uint64 fCount);
void fDArkData(char **filenames, uint64 fCount, char skip);

#endif //MLZ_FILE_H
