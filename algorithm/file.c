#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include "LZW.h"

#define uint64 unsigned long long int
#define uchar unsigned char


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

// Unzipping .mlz into a folder
char mlzGetData(const char *filename, const char *folder, uint64 folderLen) {
    uint64 fCount, dataLen;
    unsigned int nameLen;
    uchar uSize; // unit size of the component data code unit
    uchar *decoded;
    char *nameFile;
    FILE* fileMlz = fopen(filename, "rb");
    // reading file data
    // error checking
    if (ferror(fileMlz) != 0) {
        fprintf(stderr, "Error reading file. ->fLoad\n");
        return 0;
    }
    fread(&fCount, sizeof(uint64), 1, fileMlz);
    uint64 N;
    for (uint64 i = 0; i < fCount; ++i) {
        fread(&nameLen, sizeof(int), 1, fileMlz);
        nameFile = malloc(sizeof(uchar) * (nameLen + 1)); // file
        fread(nameFile, sizeof(uchar), nameLen, fileMlz); // filename
        nameFile[nameLen] = '\0';
        fread(&uSize, sizeof(uchar), 1, fileMlz); // unit size
        fread(&N, sizeof(uint64), 1, fileMlz); // number of codes
        uint64 *fileData = malloc(sizeof(uint64) * N); // file content as uint64
        for (uint64 j = 0; j < N; ++j) {
            fread(&fileData[j], uSize, 1, fileMlz); // file content
        }
        decoded = lzwDecode(fileData, N, &dataLen);
        if (decoded == NULL) {
            fprintf(stderr, "Error decoding file. ->mlzGetData (%s)\n", filename);
            return 0;
        }
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
    fclose(fileMlz);
    return 1;
}

// Prints the contents of the specified .mlz file as a list of file sizes (compressed) and file names
// Ensures that all files are .mlz files
void fGetContent(char **filenames, uint64 fCount) {
    uchar uSize, *nameFile; // unit size of the component data code unit
    uint64 fCounts, N, hash;
    unsigned int nameLen;
    for (uint64 i = 0; i < fCount; ++i) {
        // file progressing
        FILE* arcFile = fopen(filenames[i], "rb");
        if (ferror(arcFile) != 0 || arcFile == NULL) { // error checking
            fprintf(stderr, "Error reading file. ->fLoad\n");
            return;
        }
        printf("IN %s\n", filenames[i]);
        printf(".________________________________.\n");
        printf("COMPRESSED      SIZE      FILENAME\n"); // no more than 10 characters per size
        fread(&fCounts, sizeof(uint64), 1, arcFile); // file count
        for (uint64 j = 0; j < fCounts; ++j) {
            fread(&nameLen, sizeof(int), 1, arcFile);
            nameFile = malloc(sizeof(uchar) * (nameLen + 1)); // file
            fread(nameFile, sizeof(uchar), nameLen, arcFile); // filename
            nameFile[nameLen] = '\0';
            fread(&uSize, sizeof(uchar), 1, arcFile); // unit size
            fread(&N, sizeof(uint64), 1, arcFile); // number of codes
            for (uint64 v = 0; v < N; ++v) {
                fread(&hash, uSize, 1, arcFile); // file content
            }
            char ch;if (uSize == 1) {ch = '-';} else {ch = '+';}
            printf("%c               %s  %s\n", ch, sizeToStr(N * uSize), nameFile);
        }
        printf(".________________________________.\n");
        fclose(arcFile);
    }
    free(nameFile);
}

// Archiving
void fArcData(char **filenames, uint64 fCount) {
    char * arcName = uniqName();
    if (arcName[0] == '\0') {
        fprintf(stderr, "Failed to create directory\n");
        fprintf(stderr, "Failed archive\n");
        exit(1);
    }
    FILE *archive = fopen(arcName, "wb");
    fwrite(&fCount, sizeof(uint64), 1, archive); // recording the number of files
    for (uint64 i = 0; i < fCount; ++i) { // walk through files
        int nameLen = 0;
        char *fileName = getName(filenames[i], &nameLen);
        fwrite(&nameLen, sizeof(int), 1, archive); // record length file name
        fwrite(fileName, sizeof(char), nameLen, archive); // filename entry
        uint64 fileSize;
        FILE* file = fopen(filenames[i], "rb");
        // reading file size
        fseek(file, 0, SEEK_END);
        fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);
        // reading file data
        uchar *fileData = malloc(sizeof(uchar) * fileSize);
        fread(fileData, sizeof(uchar), fileSize, file);
        // error checking
        if (ferror(file) != 0) {
            fprintf(stderr, "Error reading file. ->fLoad\n");
            return;
        }
        fgetc(file);
        if (feof(file) == 0) {
            fprintf(stderr, "Failed to read file. ->fLoad\n");
            return;
        }
        fclose(file);
        uint64 encLen = 0; uchar uSize;
        uint64 *dataEnc = lzwEncode(fileData, fileSize, &encLen, &uSize); // encoded data
        // если размер сжатой больше размера исходной - сохранять без сжатия
        if ((encLen * uSize) >= fileSize) {
            uSize = 1;
            fwrite(&uSize, sizeof(uchar), 1, archive); // record unit size
            fwrite(&fileSize, sizeof(uint64), 1, archive); // recording the length of data
            fwrite(fileData, sizeof(uchar), fileSize, archive);
        } else {
            fwrite(&uSize, sizeof(uchar), 1, archive); // record unit size
            fwrite(&encLen, sizeof(uint64), 1, archive); // recording the length of encoded data
            for (uint64 j = 0; j < encLen; ++j) {
                fwrite(&dataEnc[j], uSize, 1, archive); // data recording
            }
        }
        free(dataEnc);
        free(fileData);
        free(fileName);
    }
    free(arcName);
    fclose(archive);
}

// Unzip
// Ensures that all files are .mlz files
void fDArkData(char **filenames, uint64 fCount) {
    uint64 dirSize;
    for (uint64 i = 0; i < fCount; ++i) {
        printLB(i, fCount);
        char *dirname = mkDirectory(filenames[i], &dirSize);
        if (dirname[0] == '\0') {
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