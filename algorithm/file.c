#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include "LZW.h"

#define uint64 unsigned long long int
#define uchar unsigned char

// Возвращает имя без пути и считает его длину
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

// создание папки
char *mkDirectory(const char *name) {
    char *newName = malloc(sizeof(char) * 500); int i = 0; char ch[5];
    int c = 0;
    for (; name[i] != '\0'; ++i) {newName[i] = name[i];}
    newName[i] = '\0';
    while (c < 1000 && mkdir(newName, 0777) == -1) {
        newName[i] = '_';
        for (int j = 0; j < 5; ++j) {
            newName[i + 1 + j] = ch[j];
            if (j % 2 == 0) {
                ++ch[j];
            } else {
                --ch[j];
            }
        }
        ++c;
    }
    if (c >= 1000) {
        newName[0] = '\0';
        return newName;
    }
    return newName;
}

// выбор уникального имени архива
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

// Возвращает строго 11 символов (строка длины 10)
char *sizeToStr(uint64 size){
    char syf[6][3] = {"B\0\0", "KB\0", "MB\0", "GB\0", "TB\0", "PB\0"};
    int i = 0;
    while(size >= 1024){
        size /= 1024;
        ++i;
    }
    char *str = malloc(sizeof(char) * 11);
    sprintf(str, "%llu %s", size, syf[i]);
    for (int j = 0; j < 10; ++j) {
        if (!((str[j] <= '9' && str[j] >= '0') || (str[j] <= 'Z' && str[j] >= 'A'))) { // не цифра или буква
            str[j] = ' ';
        }
    } str[11] = '\0';
    return str;
}

// Возвращает 1 если пользователь согласен продолжить без файла, иначе 0
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


// проверяет существование фала
// 0 - не существует, 1 - существует
char fCheck(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        return 0;
    }
    return 1;
}
// 1 - .mlz файл, иначе 0
char exCheck(const char *filename) {
    int i;
    for (i = 0; filename[i] != '\0'; ++i) {}
    if (filename[i - 1] == 'z' && filename[i - 2] == 'l' && filename[i - 3] == 'm' && filename[i - 4] == '.') {
        return 1;
    }
    return 0;
}

// файл будет открыт и прочитан
// принимает имя файла и указатель на размер файла
// возвращает указатель на содержимое файла
uchar *fRead(const char *filename, volatile uint64 *fSize, volatile uint64 *dSize, char mode) {
    FILE* file = fopen(filename, "r");
//     чтение размера файла
    fseek(file, 0, SEEK_END);
    *fSize = ftell(file);
    fseek(file, 0, SEEK_SET);
//     чтение данных файла
    uchar *fData = malloc(sizeof(uchar) * *fSize);
    fread(fData, sizeof(uchar), *fSize, file);
    // проверка ошибок
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
    // считать количество файлов (uint64)
    if (mode == 'a') {
        uint64 N = 0;
        uint64 p256n[8] = {1, 256, 65536, 16777216, 4294967296, 1099511627776, 281474976710656, 72057594037927936};
        for (int i = 7; i >= 0; --i) {
            N += fData[7 - i] * p256n[i];
        } // сборка uint64
        *dSize = N;
    }
    return fData;
}

// файл будет сохранен и записан
// принимает имя файла, указатель на содержимое файла и размер файла
// возвращает 0 в случае успеха, 1 в случае ошибки
char fWrite(const char *filename, uchar *data, const uint64 size) {
    FILE* file = fopen(filename, "w");
    uint64 realSize = fwrite(data, sizeof(uchar), size, file);
    if (realSize!= size) {
        fprintf(stderr, "Error writing file. ->fSave\n");
        return 1;
    }
    fclose(file);
    return 0;
}

// Возвращает список строк-имен файлов в .mlz архиве
// size - список размеров файлов в.mlz архиве. fCount - количество файлов в.mlz архиве.
uchar ** mlzGetNames(const char *filename, uint64 **size, uint64 *fCount, char skip) {
    uint64 start = 8; // начало последовательности файлов
    uint64 fSize = 0;
    uchar uSize; // размер Юнета составляющей кодовой единицы данных
    uchar *content = fRead(filename, &fSize, fCount, 'a');
    uchar **fNames = malloc(sizeof(uchar) * *fCount);
    for (int i = 0; i < *fCount; ++i) { // цикл по файлам
        if (content == NULL) {
            if (skip == 0 && askUser() == 0) {
                return NULL;
            }
        }
        unsigned int N = 0;
        uint64 p256n[8] = {1, 256, 65536, 16777216, 4294967296, 1099511627776, 281474976710656, 72057594037927936};
        for (int j = 3; j >= 0; --j) {
            N += content[start + 3 - i] * p256n[i];
        } // сборка uint длинны имени файла
        start += 4;
        fNames[i] = malloc(sizeof(uchar) * (N + 1));
        for (int j = 0; j < N; ++j) { // цикл символам имени файла
            fNames[i][j] = content[start + j];
        }
        start += N;
        uSize = content[start]; // размер юнета
        start += 1;
        N = 0;
        for (int j = 7; j >= 0; --j) {
            N += content[start + 7 - i] * p256n[i];
        } // сборка uint64 длинны длины содержимого файла
        *size[i] = N * uSize;
        start += 8 + N;
    }
    free(content);
    return fNames;
}

// Разархивация .mlz в папку
char mlzGetData(const char *filename, char *folder) {
    uint64 fCount, fSize;
    uchar *content = fRead(filename, &fSize, &fCount, 'a');


    free(content);
    return 1;
}

// Печатает содержимое указанного файла.mlz как список размеров файлов(сжатых) и имен файлов
// Гарантирует, что все файлы - файлы.mlz
void fGetContent(char **filenames, uint64 fCount, char skip) {
    uchar **file = NULL; uint64 *fSize = NULL; uint64 fCounts = 0;
    printf("SIZE      FILENAME\n"); // не более 10 символов на размер
    for (uint64 i = 0; i < fCount; i++) {
        file = mlzGetNames(filenames[i], &fSize, &fCounts, skip);
        if (file != NULL) {
            printf("IN %s\n", filenames[i]);
            for (uint64 j = 0; j < fCounts; j++) {
                printf("%s  %s\n", sizeToStr(fSize[j]), file[j]);
            }
        }
    }
    if (file!= NULL) {
        for (uint64 i = 0; i < fCount; i++) {
            if (file[i] != NULL) {
                free(file[i]);
            }
        }
        free(file);
    }
}

// Архивация
void fArcData(char **filenames, uint64 fCount, char skip) {
    char * arcName = uniqName();
    if (arcName[0] == '\0') {
        fprintf(stderr, "Failed to create directory\n");
        fprintf(stderr, "Failed archive\n");
        exit(1);
    }
    FILE *archive = fopen(arcName, "w");
    fwrite(&fCount, sizeof(uint64), 1, archive); // запись числа фалов
    for (uint64 i = 0; i < fCount; ++i) { // проход по файлам
        int nameLen = 0;
        char *fileName = getName(filenames[i], &nameLen);
        fwrite(&nameLen, sizeof(int), 1, archive); // запись длинны имени файла
        fwrite(fileName, sizeof(char), nameLen, archive); // запись имени файла
        uint64 fileSize = 0;
        uchar *fileData = fRead(filenames[i], &fileSize, 0, 'w');
        uint64 encLen = 0; uchar uSize;
        uint64 *dataEnc = lzwEncode(fileData, fileSize, &encLen, &uSize); // кодированные данные
        fwrite(&uSize, sizeof(uchar), 1, archive); // запись размера юнета
        fwrite(&encLen, sizeof(uint64), 1, archive); // запись длины кодированных данных
        if (uSize == 8) {
            fwrite(&dataEnc, sizeof(uint64), encLen, archive); // запись данных
        }
        if (uSize == 4) {
            unsigned int h;
            for (uint64 j = 0; j < encLen; ++j) {
                h = (unsigned int)dataEnc[j];
                fwrite(&h, sizeof(int), 1, archive); // запись данных
            }
        }
        free(dataEnc);
        free(fileData);
    }
    free(arcName);
    fclose(archive);
}

// Разархивация
// Гарантирует, что все файлы - файлы.mlz
void fDArkData(char **filenames, uint64 fCount, char skip) {
    for (uint64 i = 0; i < fCount; i++) {
        char *dirname = mkDirectory(filenames[i]);
        if (dirname[0] == '\0') {
            fprintf(stderr, "Failed to create directory %s\n", filenames[i]);
            fprintf(stderr, "Skipping...\n");
        } else {
            mlzGetData(filenames[i], dirname);
        }
        printLB(i, fCount);
        free(dirname);
    }
}

/*
  .mlz file content format:
byte:           8                 4                       K                      1                     8                M*P
image:   [count of files] [len of file name K][filename while char != '\0][size of code units P][size of code count M][M code units]
repeat:  |    1        | |                               N times                                                                |
 */
