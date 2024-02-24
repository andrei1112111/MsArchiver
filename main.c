#include <stdio.h>
#include <stdlib.h>

#include "algorithm/file.h"

#define uint64 unsigned long long int
#define uchar unsigned char


enum workMode {wArchiving, wDearchiving, wInfo, wNone};

// 0 в случае совпадения строк
int strcmp(const char *s1, const char *s2) {
    for (int i = 0;; ++i) {
        if (s1[i] == '\0' && s2[i] == '\0') {
            return 0;
        }
        if (s1[i]!= s2[i]) {
            return s1[i] - s2[i];
        }
    }
}


int main(int argc, char **argv) {
    char **files = NULL;
    char workMod  = wNone;
    int fileNames = -1; // Номер в списке аргументов с которого начинаются имена файлов
    int fileCount = 0; // Количество файлов в списке
    char skip = 0; // спрашивать что делать в случае ошибок
    if (argc <= 1) { // запускаем консольный режим
        printf("Welcome to .mlw file archiver\n\n"
               "1 - Create archive. 2 - Show archive content. 3 - Extract files from the archive\n");
        char inp1;
        do {
            printf("Enter the operating mode: ");
            fflush(stdout);
            scanf("%c", &inp1);
            fflush(stdin);
            switch (inp1) {
                case '1':
                    workMod = wArchiving;
                    break;
                case '2':
                    workMod = wInfo;
                    break;
                case '3':
                    workMod = wDearchiving;
                    break;
                default:
                    break;
            }
        } while (workMod == wNone);
        printf("Do you want to see warnings in case of errors?\n");
        do {
            printf("Type (yes/no): ");
            fflush(stdout);
            scanf("%s", &inp1);
            fflush(stdin);
            if (inp1 == 'y' || inp1 == 'Y') { break;}
            if (inp1 == 'n' || inp1 == 'N') { skip = 1; break;}
        } while (1);
        files = (char **) malloc(sizeof(char **) * 256);
        char *inp2 = malloc(sizeof(char) * 256);
        printf("Enter the names of the files to be archived separated by newlines:\n");
        printf("When you're done, type 'e'\n");
        scanf("%s", inp2);
        fflush(stdin);
        while (inp2[0] != 'e' && inp2[1]!= '\0') {
            if (workMod == wInfo || workMod == wDearchiving) { // проверка расширения
                if (exCheck(inp2) == 0) {
                    if (skip == 0) {
                        fprintf(stderr, "This file has an invalid extension: %s\n", inp2);
                        fflush(stderr);
                        if (askUser() == 0) {
                            return 0;
                        } else {
                            scanf("%s", inp2);
                            fflush(stdin);
                            continue; // не подошел по расширению
                        }
                    }
                }
            }
            if (fCheck(inp2) == 0) { // файл не найден
                if (skip == 0) {
                    fprintf(stderr, "This file already exists: %s\n", inp2);
                    fflush(stderr);
                    if (askUser() == 0) {
                        return 0;
                    } else {
                        scanf("%s", inp2);
                        fflush(stdin);
                        continue;
                    }
                }
            }
            int lf = 1; for (; inp2[lf] != '\0'; ++lf) {}
            files[fileCount] = malloc(sizeof(char) * (lf + 1));
            for (int i = 0; i < lf+1; ++i) { files[fileCount][i] = inp2[i];}
            ++fileCount;
            scanf("%s", inp2);
        }
        printf("\n\n\n");
    } else { // Работа с аргументами командной строки
        for (int i = 1; i < argc; ++i) { // если поступило сразу несколько команд - будет выполнена последняя из них
            if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) { // выводим справку
                printf("Usage:\n"
                       "PARAMETER         DESCRIPTION                                                           \n"
                       "-h, --help        This help. (only this one will be printed)                            \n"
                       "-f, --file         Input file. Multiple files separated by a space are allowed             \n"
                       "-e, --encode      Input files will be compressed (Files with the .mlz extension will be  \n"
                       "                                             compressed. The rest will remain untouched)\n"
                       "-d, --decode      Input files will be decompressed                                       \n"
                       "-c, --check       Returns the contents of the archives. (works only for .mlz files)      \n"
                       "-s, --skip        Skip compression/decompression for all files with errors. (warns about \n"
                       "                                                                      errors by default)\n");
                return 0;
            }
            if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--check") == 0) { // вернуть содержимое архива
                workMod = wInfo;
            }
            if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--file") == 0) { // добавляем файл
                fileNames = i+1;
            }
            if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--encode") == 0) { // режим кодирования
                workMod = wArchiving;
            }
            if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--decode") == 0) { // режим декодирования
                workMod = wDearchiving;
            }
            if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--skip") == 0) { // пропускать кодирование/декодирование для всех файлов с ошибками
                skip = 1;
            }
        }
        // исключение ошибок
        if (workMod == wNone) {
            fprintf(stderr, "Operating mode was not specified\n");
            exit(1);
        }
        files = (char **) malloc(sizeof(char **) * argc);
        for (int i = fileNames; i < argc; ++i) {
            if (fCheck(argv[i]) == 0) {
                if (skip == 0) {
                    fprintf(stderr, "This file was not found: %s\n", argv[i]);
                    if (askUser() == 0) {
                        return 0;
                    } else {
                        continue; // не найден
                    }
                }
            } else {
                if (workMod == wInfo || workMod == wDearchiving) { // проверка расширения
                    if (exCheck(argv[i]) == 0) {
                        if (skip == 0) {
                            fprintf(stderr, "This file has an invalid extension: %s\n", argv[i]);
                            if (askUser() == 0) {
                                return 0;
                            } else {
                                continue; // не подошел по расширению
                            }
                        }
                    }
                }
                int lf = 0; for (; argv[fileCount][lf] != '\0'; ++lf) {}
                files[fileCount] = malloc(sizeof(char) * (lf + 1));
                for (int ii = 0; ii < lf+1; ++ii) { files[fileCount][i] = argv[fileCount][i];}
                ++fileCount;
            }
        }
        if (fileCount == 0) {
            fprintf(stderr, "No files were specified\n");
            exit(1);
        }
    }
    // работа с данными пользователя
    if (workMod == wArchiving) { // архивация
        fArcData(files, fileCount);
    }
    if (workMod == wDearchiving) { // разархивация
        fDArkData(files, fileCount, skip);
    }
    if (workMod == wInfo) { // проверка содержимого архива
        fGetContent(files, fileCount, skip);
    }
    printf("\nComplete\n");
    return 0;
}
// докрутить безопасности на всем проекте (защита от переполнения типов)

// !!! test_files/1.txt
// !!! archive.mlz
