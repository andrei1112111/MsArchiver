#include <stdio.h>
#include <stdlib.h>

#include "algorithm/LZW.h"


int main(void) {
    unsigned long long int size;
    unsigned long long int len_encoded, len_decoded;
// --------------------------
    FILE* file = fopen("2638_27500.txt", "r");
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    unsigned char *input = malloc(size);
    fread(input, sizeof(char), size, file);
    fclose(file);
// --------------------------
    unsigned long long int *encoded = (unsigned long long int *) lzw_encode(input, size, &len_encoded);
//    return 0;
//    for (int i = 0; i < len_encoded; ++i) {printf("%llu ", encoded[i]);}
    printf("Длинна исходной(байт): %llu. Длинна сжатой('чисел'): %llu\n", size, len_encoded);

    unsigned char *decoded = lzw_decode(encoded, len_encoded, &len_decoded);

//    for (unsigned int i = 0; i < len_encoded; ++i) { printf("%llu ", encoded[i]); } printf("\n");
//    for (unsigned int i = 0; i < len_decoded; ++i) { printf("%c", decoded[i]); } printf("\n");

    printf("Символов: %llu\n", len_decoded);
    printf("Исходная == Декодированная: ");
    int h = 1;
    if (size == len_decoded) {
        for (unsigned int i = 0; i < len_decoded; ++i) {
            if (input[i]!= decoded[i]) {
                printf("\n%d != %d (%d) |%c|\n", input[i], decoded[i], i, input[i]);
                h = 0;
                break;
            }
        }
    } else {h = 0;}
    if (h == 1) {puts("Да\n");} else {puts("Нет\n");}
// --------------------------
    free(encoded);
    free(decoded);
    return 0;
}
