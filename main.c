#include <stdio.h>
#include <stdlib.h>



// проверка включения последовательности в 'словарь'
int stc_in_dict (const unsigned char *str, unsigned char ch, unsigned char **dict, unsigned int dict_len, unsigned int str_len) {
    // строкой названа последовательность байтов
    if (str_len == 0) {return 1;}
    for (unsigned int i = 256; i < dict_len; ++i) {
        for (unsigned int j = 0; j <= str_len; ++j) {
            if (dict[i][0] == (str_len + 1)) {
                // совпало
                if (j == str_len && ch == dict[i][j+1]) {
                    return 1;
                }
                // очевидно различны
                if (str[j] != dict[i][j+1]) {
                    break;
                }
            }
        }
    }
    return 0;
}

unsigned int get_dict(unsigned char **dict, unsigned int dict_len, unsigned char *str, unsigned int str_len) {
    if (str_len == 1) {return str[0];}
    for (unsigned int i = 256; i < dict_len; ++i) {
        for (unsigned int j = 0; j < str_len; ++j) {
            if (dict[i][0] == str_len) {
                // очевидно различны
                if (str[j] != dict[i][j+1]) {
                    break;
                }
                // совпало
                if ((j+1) == str_len) {
                    return i;
                }
            }
        }
    }
    return 0;
}


// lzw для байтовых последовательностей (сжатие)
// преобразование списка байтов в список некоторых 'чисел'
unsigned int *lzw_encode(unsigned char *input, size_t size_inp, unsigned long long int *result_len) {
    // словарь состоит из длины (пока что первый байт) и самой последовательности далее
    unsigned int len_key = 0; unsigned int key_mem_step = 4096; unsigned int mem_for_key = 4096;
    unsigned char *key = malloc(sizeof(char ) * mem_for_key); // определение 'строки'-ключа

    unsigned int dict_len = 256; unsigned int dict_mem_step = 4096; unsigned int mem_for_dict = 4096;
    unsigned char **dict = malloc(sizeof(unsigned char *) * mem_for_dict); // определение 'словаря'
    // заполнение словаря всевозможными значениями
    for (int i = 0; i < 256; ++i) { dict[i] = malloc(sizeof(char ) * 2) ; dict[i][0] = 1; dict[i][1] = (char )i;}

    // результат (непонятно сколько памяти держать под каждое значение (типа тип инта))
    unsigned int res_len = 0; unsigned int res_mem_step = 4096; unsigned int mem_for_res = 4096;
    unsigned int *res = malloc(sizeof(unsigned int) * mem_for_res); // сжатая последовательность

    for (unsigned int inp_byte = 0; inp_byte < size_inp; ++inp_byte) { // проход по байтам 'input'
        if (stc_in_dict(key, input[inp_byte], dict, dict_len, len_key) == 1) { // (w+c) in dict
            if ((len_key + 1) > mem_for_key) { // память под ключ кончилась
                mem_for_key = mem_for_key + key_mem_step;
                key = realloc(key, mem_for_key * sizeof(char ));
                if (key == NULL) {
                    fprintf(stderr, "Memory allocation error\n");
                    exit(1);
                }
            }
            // key = key + input[inp_byte]
            key[len_key] = input[inp_byte];
            ++len_key;
        } else {
            if ((dict_len + 1) > mem_for_dict) { // память под словарь кончилась
                mem_for_dict = mem_for_dict + dict_mem_step;
                dict = (unsigned char **) realloc(dict, mem_for_dict * sizeof(unsigned char *));
//                printf("dict: %d\n", mem_for_dict);
                if (dict == NULL) {
                    fprintf(stderr, "Memory allocation error\n");
                    exit(1);
                }
            }
            if ((res_len + 1) > mem_for_res) { // память под результат кончилась
                mem_for_res = mem_for_res + res_mem_step;
                res = realloc(res, mem_for_res * sizeof(unsigned int));
                if (res == NULL) {
                    fprintf(stderr, "Memory allocation error\n");
                    exit(1);
                }
            }
            // result.append(dictionary[w])
            res[res_len] = get_dict(dict, dict_len, key, len_key);
            if (res[res_len] > 65536) {printf("ok\n"); exit(1);} // хватит два байта
            ++res_len;
            // dict[dict_len] = (key+input[inp_byte])
            dict[dict_len] = malloc(sizeof(char ) * (len_key + 2));
            for (unsigned int i = 0; i < len_key; ++i) {dict[dict_len][i+1] = key[i];}
            dict[dict_len][len_key + 1] = input[inp_byte]; dict[dict_len][0] = (unsigned char )(len_key + 1); // типа длинна в нулевом байте
            // key = input[inp_byte]
            ++dict_len;
            key[0] = input[inp_byte]; len_key = 1;
        }
    }
    if (size_inp > 0) {
        if ((res_len + 1) > mem_for_res) { // память под результат кончилась
            mem_for_res = mem_for_res + res_mem_step;
            res = realloc(res, mem_for_res * sizeof(unsigned int));
            if (res == NULL) {
                fprintf(stderr, "Memory allocation error\n");
                exit(1);
            }
        }
        // result.append(dictionary[w])
        res[res_len] = get_dict(dict, dict_len, key, len_key);
        ++res_len;
    }
    *result_len = res_len;
    for (unsigned int i = 0; i < dict_len; ++i) {
        free(dict[i]);
    } printf("\n");
    free(dict);
    free(key);
    return res;
}


int main() {
    unsigned long long int size;
    unsigned long long int len_result;
    // --------------------------
    FILE* file = fopen("2638.txt", "r");
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    unsigned char *input = malloc(size);
    fread(input, sizeof(char), size, file);
    fclose(file);

    unsigned int *result = lzw_encode(input, size, &len_result);

    printf("Длинна исходной(байт): %llu. Длинна сжатой('чисел'): %llu\n", size, len_result);
    free(result);
    return 0;
}




// 11.02 -- 90!
// 12.02 -- 150! рабочий алгоритм кодирования LZW на вложенном массиве (22336391857 инструкций; 1,85 мб; 2.16 с; 2638.txt; Исходно байт: 50656; Выход байт: [14533*2])
//               алгоритм на префиксном дереве
