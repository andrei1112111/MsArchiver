#include <stdlib.h>
#include <stdio.h>

#define uint64 unsigned long long int
#define uchar unsigned char


// файл будет открыт и прочитан
// принимает имя файла и указатель на размер файла
// возвращает указатель на содержимое файла
uchar *fLoad(const char *filename, volatile uint64 *size) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) { // файл не найден
        fprintf(stderr, "No such file exists. ->fLoad\n");
        return NULL;
    }
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
    };
    fclose(file);
    return fData;
}

// файл будет сохранен и записан
// принимает имя файла, указатель на содержимое файла и размер файла
// возвращает 0 в случае успеха, 1 в случае ошибки
char fSave(const char *filename, volatile uchar *data, const uint64 size) {
    FILE* file = fopen(filename, "w");
    uint64 realSize = fwrite(data, sizeof(uchar), size, file);
    if (realSize!= size) {
        fprintf(stderr, "Error writing file. ->fSave\n");
        return 1;
    }
    fclose(file);
    return 0;
}
