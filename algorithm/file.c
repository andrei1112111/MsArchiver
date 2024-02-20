#include <stdlib.h>
#include <stdio.h>

#define uint64 unsigned long long int
#define uchar unsigned char


// Возвращает 1 если пользователь согласен продолжить без файла, иначе 0
char askUser(void) {
    while (1) {
        printf("Continue without this file?\n");
        printf("Type (yes/no): ");
        fflush(stdout);
        char answer[1];
        scanf("%s", answer);
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
uchar *fRead(const char *filename, volatile uint64 *size) {
    if (fCheck(filename) == 0) { // файл не найден
        fprintf(stderr, "No such file exists. ->fLoad\n");
        return NULL;
    }
    FILE* file = fopen(filename, "r");
//     чтение размера файла
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);
//     чтение данных файла
    uchar *fData = malloc(sizeof(uchar) * *size);
    fread(fData, sizeof(uchar), *size, file);
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
uchar ** mlzGetContent(const char *filename, uint64 *length) {

    return NULL;
}

// Возвращает содержимое указанного файла .mlz как список кодовых последовательностей файлов
uint64 * mlzGetData(const char *filename, uint64 *length) {

    return NULL;
}

// Сохраняет файлы в.mlz архив
uchar * mlzSaveData(const char **filenames, uint64 *length) {

    return NULL;
}

// Возвращает содержимое указанного файла.mlz как список размеров файлов(сжатых) и имен файлов
void fGetContent(char **filenames, uint64 fCount, char skip) {
    for (uint64 i = 0; i < fCount; i++) {

    }
}

// Архивация
void fArcData(char **filenames, uint64 fCount, char skip) {

}

// Разархивация
void fDArkData(char **filenames, uint64 fCount, char skip) {

}

/*
  .mlz file content format:
byte:           8                      _                     1                   1                  M            V
image:   [count of files] [filename while char != '\0][size of code units][size of code count M][code count V][code units]
repeat:  |    1        | |                               N times                                                       |
 */
