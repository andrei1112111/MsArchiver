#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include "LZW.h"

#define uint64 unsigned long long int
#define uchar unsigned char

struct file {
    uchar *name;
    uint64 size;
};

// convert 8 chars to uint64
uint64 convert(const uchar* input) {
    return *((uint64*)input);
}

// returns 1 when the input string is a file
char isFile(const char *filename) {
    if (filename[0] == '-') {return 0;}
    for (int i = 0; filename[i] != '\0'; ++i) {
        if (filename[i] == '.' && filename[i + 1] != '\0') {
            return 1;
        }
    }
    return 0;
}

// Returns the name without the path and calculates its length
char *getName(const char *filename, int *len) {
    int i,j = 0;
    for (i = 0; filename[i] != '\0'; ++i) {
        if (filename[i] == '/') {
            j = i + 1;
        }
    }
    char *name = malloc(sizeof(char) * (i - j + 1));
    for (i = j; filename[i]!= '\0'; ++i) {
        name[i - j] = filename[i];
    }
    name[i - j] = '\0';
    *len = i - j;
    return name;
}

// creating a folder with unique name
char *mkDirectory(const char *name, uint64 *folderLen) {
    int i = 0;
    for (; name[i] != '\0'; ++i) {}
    i -= 4;
    char *newName = malloc(sizeof(char) * (i + 2 + 6));
    for (int j = 0; j < i; ++j) {newName[j] = name[j];}
    newName[i] = '\0';
    char ch[6] = "000000"; int c = 0;
    while (c < 1000 && mkdir(newName, 0777) == -1) {
        newName[i] = '_';
        for (int j = 0; j < 5; ++j) {
            newName[i + 1 + j] = ch[j];
        }
        newName[i + 1 + 5] = '\0';
        ++ch[0];
        for (int z = 0; z < 5; ++z) {
            if (ch[z] > '9') {
                ch[z] = 0;
                ch[z + 1]++;
            }
        }
        ++c;
    }
    if (c >= 1000) {
        newName[0] = '\0';
        return newName;
    }
    if (c == 0) {
        *folderLen = i;
    } else {
        *folderLen = i + 1 + 5;
    }
    return newName;
}

// choosing a unique name for the archive
char *uniqName(void) {
    char *filename = malloc(sizeof(char) * 100);
    for (int i = 0; i < 13; ++i) {filename[i] = "archive.mlz\0"[i];}
    char ch[6] = "000000"; int c = 0;
    FILE* file = fopen(filename, "r");
    while (c < 1000 && file!= NULL) {
        filename[7] = '_';
        for (int j = 0; j < 5; ++j) {
            filename[7 + 1 + j] = ch[j];
        }
        ++ch[0];
        for (int z = 0; z < 5; ++z) {
            if (ch[z] > '9') {
                ch[z] = 0;
                ch[z + 1]++;
            }
        }
        ++c;
        filename[13] = '.'; filename[14] = 'm'; filename[15] = 'l'; filename[16] = 'z'; filename[17] = '\0';
        file = fopen(filename, "r");
    }
    if (c >= 1000) {
        filename[0] = '\0';
        return filename;
    }
    return filename;
}

// печатает текущий статус обработки (10 степеней)
void printLB(uint64 cur, uint64 end) {
    char st = (char) (cur / end * 10); // [0, ..., 10]
    printf("|");
    for (char j = 0; j < st; ++j) {
        printf("#");
    }
    for (char j = st; j < 10; ++j) {
        printf(" ");
    }
    printf("| %llu/%llu\r", cur, end);
    fflush(stdout);
}

// Converts file size in bytes to kb mb ...
// Returns 11 characters (string length = 10)
char *sizeToStr(uint64 size){
    char syf[6][3] = {"B\0\0", "KB\0", "MB\0", "GB\0", "TB\0", "PB\0"};
    int i = 0;
    while(size >= 1024){
        size /= 1024;
        ++i;
    }
    char *str = malloc(sizeof(char) * 11);
    sprintf(str, "%llu %s", size, syf[i]);
    for (int j = 0; j < 8; ++j) {
        if (!((str[j] <= '9' && str[j] >= '0') || (str[j] <= 'Z' && str[j] >= 'A'))) { // не цифра или буква
            str[j] = ' ';
        }
    } str[11] = '\0';
    return str;
}

// Returns 1 if the user agrees to continue without the file, otherwise 0
char askUser(void) {
    while (1) {
        char answer[1];
        printf("Continue without this file?\n");
        printf("Type (yes/no): ");
        fflush(stdout);
        scanf("%s", answer);
        fflush(stdin);
        if (answer[0] == 'y' || answer[0] == 'Y') {return 1;}
        if (answer[0] == 'n' || answer[0] == 'N') {return 0;}
    }
}


// Checks File Existence
// 0 - does not exist, 1 - exists
char fCheck(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        return 0;
    }
    return 1;
}
// 1 - .mlz archive, otherwise 0
char exCheck(const char *filename) {
    int i;
    for (i = 0; filename[i] != '\0'; ++i) {}
    if (filename[i - 1] == 'z' && filename[i - 2] == 'l' && filename[i - 3] == 'm' && filename[i - 4] == '.') {
        return 1;
    }
    return 0;
}

// the file will be opened and read
// accepts the file name, a pointer to the file size, a pointer to the number of files in the archive and the operating mode
// returns a pointer to the contents of the file
uchar *fRead(const char *filename, volatile uint64 *fSize, volatile uint64 *dSize, char mode) {
    FILE* file = fopen(filename, "r");
    // reading file size
    fseek(file, 0, SEEK_END);
    *fSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    // reading file data
    uchar *fData = malloc(sizeof(uchar) * *fSize);
    fread(fData, sizeof(uchar), *fSize, file);
    // error checking
    if (ferror(file) != 0) {
        fprintf(stderr, "Error reading file. ->fLoad\n");
        return NULL;
    }
    fgetc(file);
    if (feof(file) == 0) {
        fprintf(stderr, "Failed to read file. ->fLoad\n");
        return NULL;
    }
    fclose(file);
    // count number of files (uint64)
    if (mode == 'a') {
        uchar N[8] = {fData[0], fData[1], fData[2], fData[3], fData[4], fData[5], fData[6], fData[7]}; // assembly uint64
        *dSize = convert(N);
    }
    return fData;
}

// Returns a list of file name strings in the .mlz archive
// size - list of file sizes in the .mlz archive. fCount - number of files in the .mlz archive.
struct file *mlzGetNames(const char *filename, uint64 *fCount, char skip) {
    uint64 start = 8; // start of file sequence
    uint64 fSize = 0;
    uchar uSize; // unit size of the component data code unit
    uchar *content = fRead(filename, &fSize, fCount, 'a');
    struct file *Files = malloc(sizeof(struct file) * (*fCount));
    for (int i = 0; i < *fCount; ++i) { // cycle through files
        if (content == NULL) {
            if (skip == 0 && askUser() == 0) {
                return NULL;
            }
        } else {
            unsigned int N;
            N = (content[start + 3] << 24) | (content[start + 2] << 16) | (content[start + 1] << 8) | (content[start]); // assembly uint length of file name
            start += 4;
            Files[i].name = malloc(sizeof(uchar) * (N + 1));
            for (int j = 0; j < N; ++j) { // loop on file name characters
                Files[i].name[j] = content[start + j];
            }
            Files[i].name[N] = '\0';
            start += N;
            uSize = content[start]; // unit size
            start += 1;
            uchar M[8] = {content[start + 0], content[start + 1], content[start + 2], content[start + 3],
                          content[start + 4], content[start + 5], content[start + 6], content[start + 7]}; // assembly uint64
            N = convert(M);
            Files[i].size = N * uSize;
            start += 8 + Files[i].size;
        }
    }
    free(content);
    return Files;
}

// Unzipping .mlz into a folder
char mlzGetData(const char *filename, const char *folder, uint64 folderLen) {
    uint64 fCount, fSize, dataSize, dataLen, nameLen;
    uchar uSize; // unit size of the component data code unit
    uint64 start = 8; // start of file sequence
    uchar *decoded;
    char *nameFile;
    uchar *content = fRead(filename, &fSize, &fCount, 'a');
    if (content == NULL) {
        fprintf(stderr, "Error reading file. ->mlzGetData (%s)\n", filename);
        return 0;
    } else {
        unsigned int N;
        for (uint64 i = 0; i < fCount; ++i) {
            N = (content[start + 3] << 24) | (content[start + 2] << 16) | (content[start + 1] << 8) | (content[start]); // assembly uint length of file name
            start += 4;
            nameLen = (N + 1);
            nameFile = malloc(sizeof(uchar) * nameLen); // archive file
            for (int j = 0; j < N; ++j) { // loop on file name characters
                nameFile[j] = (char)content[start + j];
            }
            nameFile[N] = '\0';
            start += N;
            uSize = content[start]; // unit size
            start += 1;
            uchar M[8] = {content[start + 0], content[start + 1], content[start + 2], content[start + 3],
                          content[start + 4], content[start + 5], content[start + 6], content[start + 7]}; // assembly uint64
            N = convert(M);
            dataSize = N * uSize;
            start += 8;
            // -------- to change |
            uint64 *fileData = malloc(sizeof(uint64) * dataSize); // file content as uint64
            uint64 kk = 0;
            if (uSize == 4) {
                for (;kk < N; ++kk) {
                    fileData[kk] = (content[start + (kk * uSize) + 3] << 24) | (content[start + (kk * uSize) + 2] << 16) | (content[start + (kk * uSize) + 1] << 8) | (content[start + (kk * uSize)]);
                }
            } else {
                for (; kk < N; ++kk) {
                    uchar MM[8] = {content[start + kk + 0], content[start + kk + 1], content[start + kk + 2], content[start + kk + 3],
                                  content[start + kk + 4], content[start + kk + 5], content[start + kk + 6], content[start + kk + 7]}; // assembly uint64
                    fileData[kk] = convert(MM);
                    }
                }
            decoded = lzwDecode(fileData, N, &dataLen);
            // -------- to change /
            start += (N * uSize);
            if (decoded == NULL) {
                fprintf(stderr, "Error decoding file. ->mlzGetData (%s)\n", filename);
                return 0;
            }
            // write to file
            char *path = malloc(sizeof(char) * (folderLen + nameLen + 2));
            for (int k = 0; k < folderLen; ++k) {
                path[k] = folder[k];
            }
            path[folderLen] = '/';
            for (int k = 0; k < nameLen ; ++k) {
                path[folderLen + 1 + k] = nameFile[k];
            }
            path[folderLen + 1 + nameLen] = '\0';
            FILE *file = fopen(path, "w");
            fwrite(decoded, sizeof(uchar), dataLen, file);
            fclose(file);
            free(decoded);
            free(path);
            free(nameFile);
        }
        free(content);
    }
    return 1;
}

// Prints the contents of the specified .mlz file as a list of file sizes (compressed) and file names
// Ensures that all files are .mlz files
void fGetContent(char **filenames, uint64 fCount, char skip) {
    struct file *Files = NULL; uint64 fCounts = 0;
    for (uint64 i = 0; i < fCount; ++i) {
        Files = mlzGetNames(filenames[i], &fCounts, skip);
        if (Files != NULL) {
            printf("IN %s\n", filenames[i]);
            printf(".___________________.\n");
            printf("SIZE      FILENAME\n"); // no more than 10 characters per size
            for (uint64 j = 0; j < fCounts; ++j) {
                printf("%s  %s\n", sizeToStr(Files[j].size), Files[j].name);
            }
            printf(".___________________.\n");
        }
        free(Files);
    }
}

// Archiving
void fArcData(char **filenames, uint64 fCount) {
    char * arcName = uniqName();
    if (arcName[0] == '\0') {
        fprintf(stderr, "Failed to create directory\n");
        fprintf(stderr, "Failed archive\n");
        exit(1);
    }
    FILE *archive = fopen(arcName, "w");
    fwrite(&fCount, sizeof(uint64), 1, archive); // recording the number of files
    for (uint64 i = 0; i < fCount; ++i) { // walk through files
        int nameLen = 0;
        char *fileName = getName(filenames[i], &nameLen);
        fwrite(&nameLen, sizeof(int), 1, archive); // record length file name
        fwrite(fileName, sizeof(char), nameLen, archive); // filename entry
        uint64 fileSize = 0;
        uchar *fileData = fRead(filenames[i], &fileSize, 0, 'w');
        uint64 encLen = 0; uchar uSize;
        printf("encoding\n");
        uint64 *dataEnc = lzwEncode(fileData, fileSize, &encLen, &uSize); // encoded data
        printf("encoded\n");
        fwrite(&uSize, sizeof(uchar), 1, archive); // record unit size
        fwrite(&encLen, sizeof(uint64), 1, archive); // recording the length of encoded data
        if (uSize == 8) {
            fwrite(&dataEnc, sizeof(uint64), encLen, archive); // data recording
        }
        if (uSize == 4) {
            unsigned int h;
            for (uint64 j = 0; j < encLen; ++j) {
                h = (unsigned int)dataEnc[j];
                fwrite(&h, sizeof(int), 1, archive); // data recording
            }
        }
        free(dataEnc);
        free(fileData);
    }
    free(arcName);
    fclose(archive);
}

// Unzip
// Ensures that all files are .mlz files
void fDArkData(char **filenames, uint64 fCount, char skip) {
    uint64 dirSize;
    for (uint64 i = 0; i < fCount; ++i) {
        printLB(i, fCount);
        char *dirname = mkDirectory(filenames[i], &dirSize);
        if (dirname[0] == '\0' && skip == 0) {
            fprintf(stderr, "Failed to create directory %s\n", filenames[i]);
            fprintf(stderr, "Skipping...\n");
        } else {
            mlzGetData(filenames[i], dirname, dirSize);
        }
        free(dirname);
    }
    printLB(fCount, fCount);
}

/*
  .mlz file content format:
byte:           8                 4                       K                      1                     8                M*P
image:   [count of files] [len of file name K][filename while char != '\0][size of code units P][size of code count M][M code units]
repeat:  |    1        | |                               N times                                                                |
*/