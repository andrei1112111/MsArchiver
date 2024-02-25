#include <stdio.h>
#include <stdlib.h>

#include "algorithm/file.h"

#define uint64 unsigned long long int
#define uchar unsigned char


enum workMode {wArchiving, wDearchiving, wInfo, wNone};

// 0 if strings match
int strcmp(const char *s1, const char *s2) {
    for (int i = 0;; ++i) {
        if (s1[i] == '\0' && s2[i] == '\0') {
            return 0;
        }
        if (s1[i]!= s2[i]) {
            return 1;
        }
    }
}


int main(int argc, char **argv) {
    char **files = NULL;
    char workMod  = wNone;
    int fileNames = -1; // The number in the argument list at which file names begin
    int fileCount = 0; // Number of files in the list
    char skip = 0; // ask what to do in case of errors
    if (argc <= 1) { // launch console mode
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
        files = (char **) malloc(sizeof(char **) * 257);
        char *inp2 = malloc(sizeof(char) * 257);
        printf("Enter the names of the files to be archived separated by newlines(no more than 256):\n");
        printf("When you're done, type 'e'\n");
        scanf("%s", inp2);
        fflush(stdin);
        while (inp2[0] != 'e' && inp2[1]!= '\0') {
            if (workMod == wInfo || workMod == wDearchiving) { // extension check
                if (exCheck(inp2) == 0) {
                    if (skip == 0) {
                        fprintf(stderr, "This file has an invalid extension: %s\n", inp2);
                        fflush(stderr);
                        if (askUser() == 0) {
                            return 0;
                        } else {
                            scanf("%s", inp2);
                            fflush(stdin);
                            continue; // did not fit the expansion
                        }
                    }
                }
            }
            if (fCheck(inp2) == 0) { // file not found
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
    } else { // working with command line arguments
        for (int i = 1; i < argc; ++i) { // if several commands are received at once, the last one will be executed
            if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) { // выводим справку
                printf("Usage:\n"
                       "PARAMETER         DESCRIPTION                                                           \n"
                       "-h, --help        This help. (only this one will be printed)                            \n"
                       "-f, --file        Input file. Multiple files separated by a space are allowed             \n"
                       "-e, --encode      Input files will be compressed (Files with the .mlz extension will be  \n"
                       "                                             compressed. The rest will remain untouched)\n"
                       "-d, --decode      Input files will be decompressed                                       \n"
                       "-c, --check       Returns the contents of the archives. (works only for .mlz files)      \n"
                       "-s, --skip        Skip compression/decompression for all files with errors. (warns about \n"
                       "                                                                      errors by default)\n");
                return 0;
            }
            if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--check") == 0) { // return archive contents
                workMod = wInfo;
            }
            if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--file") == 0) { // add the file
                fileNames = i+1;
            }
            if (strcmp(argv[i], "-e") == 0 || strcmp(argv[i], "--encode") == 0) { // encoding mode
                workMod = wArchiving;
            }
            if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--decode") == 0) { // decoding mode
                workMod = wDearchiving;
            }
            if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--skip") == 0) { // skip encoding/decoding for all files with errors
                skip = 1;
            }
        }
        // error elimination
        if (workMod == wNone) {
            fprintf(stderr, "Operating mode was not specified\n");
            exit(1);
        }
        if (fileNames == -1) {
            fprintf(stderr, "File names were not specified\n");
            exit(1);
        }
        files = malloc(sizeof(char *) * argc);
        for (int i = fileNames; i < argc; ++i) {
            if (isFile(argv[i]) == 0) {
                continue;
            }
            if (fCheck(argv[i]) == 0) {
                if (skip == 0) {
                    fprintf(stderr, "This file was not found: %s\n", argv[i]);
                    if (askUser() == 0) {
                        return 0;
                    } else {
                        continue; // not found
                    }
                }
            } else {
                if (workMod == wInfo || workMod == wDearchiving) { // extension check
                    if (exCheck(argv[i]) == 0) {
                        if (skip == 0) {
                            fprintf(stderr, "This file has an invalid extension: %s\n", argv[i]);
                            if (askUser() == 0) {
                                return 0;
                            } else {
                                continue; // did not fit the expansion
                            }
                        }
                    }
                }
                int lf = 0; for (; argv[i][lf] != '\0'; ++lf) {}
                files[i-fileNames] = malloc(sizeof(char) * (lf + 1));
                for (int ii = 0; ii < lf+1; ++ii) { files[i-fileNames][ii] = argv[i][ii];}
                ++fileCount;
            }
        }
        if (fileCount == 0) {
            fprintf(stderr, "No files were specified\n");
            exit(1);
        }
    }
    // working with user data
    if (workMod == wArchiving) {
        fArcData(files, fileCount);
    }
    if (workMod == wDearchiving) {
        fDArkData(files, fileCount, skip);
    }
    if (workMod == wInfo) {
        fGetContent(files, fileCount, skip);
    }
    printf("\nComplete\n");
    return 0;
}

/*
1.png
21.pdf
2638_27500.txt
e

archive.mlz
e

*/

// Tasks:
// 1. more variants for uSuze in file.c and LZW.c
