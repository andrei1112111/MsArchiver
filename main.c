#include <stdio.h>
#include <stdlib.h>

#include "algorithm/file.h"

#define uint64 unsigned long long int
#define uchar unsigned char


enum workMode {wArchiving, wDearchiving, wInfo, wNone};

// 1 if strings match
int str_cmp(const char *s1, const char *s2) {
    for (int i = 0;; ++i) {
        if (s1[i] == '\0' && s2[i] == '\0') {
            return 1;
        }
        if (s1[i]!= s2[i]) {
            return 0;
        }
    }
}

int b_in_bs(const char *b, char **bs, int bsLen) {
    for (int i = 0; i < bsLen; ++i) {
        if (str_cmp(b, bs[i]) == 1) {
            return 1;
        }
    }
    return 0;
}


int main(int argc, char **argv) {
    char **files = NULL;
    char workMod  = wNone;
    int fileNames = -1; // The number in the argument list at which file names begin
    int fileCount = 0; // Number of files in the list
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
        int filesMem = 256;
        files = (char **) malloc(sizeof(char **) * filesMem);
        char *inp2 = malloc(sizeof(char) * 4096);
        printf("Enter the names of the files to be archived separated by newlines:\n");
        printf("When you're done, type 'e'\n");
        scanf("%[^\n]", inp2);
        fflush(stdin);
        while (inp2[0] != 'e' && inp2[1]!= '\0') {
            if (fileCount > filesMem-2) {
                filesMem += 256;
                files = (char **)realloc(files,  filesMem * sizeof(char)); // NOLINT(*-suspicious-realloc-usage)
                if (files == NULL) {
                    printf("Memory allocation error\n");
                    exit(1);
                }
            }
            if (workMod == wInfo || workMod == wDearchiving) { // extension check
                if (exCheck(inp2) == 0) {
                    fprintf(stderr, "This file has an invalid extension: %s\n", inp2);
                    fflush(stderr);
                    scanf("%[^\n]", inp2);
                    fflush(stdin);
                    continue; // did not fit the expansion
                }
            }
            if (fCheck(inp2) == 0) { // file not found
                fprintf(stderr, "This file already exists: %s\n", inp2);
                fflush(stderr);
                scanf("%[^\n]", inp2);
                fflush(stdin);
                continue;
            }

            if (b_in_bs(inp2, files, fileCount) == 1) {// new file already in files
                fprintf(stderr, "already added\n");
                fflush(stderr);
                scanf("%[^\n]", inp2);
                fflush(stdin);
                continue;
            }
            int lf = 1; for (; inp2[lf] != '\0'; ++lf) {}
            files[fileCount] = malloc(sizeof(char) * (lf + 1));
            for (int i = 0; i < lf+1; ++i) { files[fileCount][i] = inp2[i];}
            ++fileCount;
            scanf("%[^\n]", inp2);
            fflush(stdin);
        }
    } else { // working with command line arguments
        for (int i = 1; i < argc; ++i) { // if several commands are received at once, the last one will be executed
            if (str_cmp(argv[i], "-h") == 1 || str_cmp(argv[i], "--help") == 1) { // выводим справку
                printf("Usage:\n"
                       "PARAMETER         DESCRIPTION                                                           \n"
                       "-h, --help        This help. (only this one will be printed)                            \n"
                       "-f, --file        Input file. Multiple files separated by a space are allowed             \n"
                       "-e, --encode      Input files will be compressed (Files with the .mlz extension will be  \n"
                       "                                             compressed. The rest will remain untouched)\n"
                       "-d, --decode      Input files will be decompressed                                       \n"
                       "-c, --check       Returns the contents of the archives. (works only for .mlz files)      \n");
                return 0;
            }
            if (str_cmp(argv[i], "-c") == 1 || str_cmp(argv[i], "--check") == 1) { // return archive contents
                workMod = wInfo;
            }
            if (str_cmp(argv[i], "-f") == 1 || str_cmp(argv[i], "--file") == 1) { // add the file
                fileNames = i+1;
            }
            if (str_cmp(argv[i], "-e") == 1 || str_cmp(argv[i], "--encode") == 1) { // encoding mode
                workMod = wArchiving;
            }
            if (str_cmp(argv[i], "-d") == 1 || str_cmp(argv[i], "--decode") == 1) { // decoding mode
                workMod = wDearchiving;
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
            if (fCheck(argv[i]) == 0) {
                fprintf(stderr, "This file was not found: %s\n", argv[i]);
                continue; // not found
            } else {
                if (workMod == wInfo || workMod == wDearchiving) { // extension check
                    if (exCheck(argv[i]) == 0) {
                        fprintf(stderr, "This file has an invalid extension: %s\n", argv[i]);
                        continue; // did not fit the expansion
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
        fDArkData(files, fileCount);
    }
    if (workMod == wInfo) {
        fGetContent(files, fileCount);
    }
    printf("Complete\n");
    return 0;
}

// compression without max vertex size for 32mb .txt file takes 25 seconds | resulting size = 10 mb
