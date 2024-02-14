#include <stdio.h>
#include <stdlib.h>


// проверка включения последовательности в 'словарь'
int stc_in_dict (const unsigned char *str, unsigned char ch, unsigned char **dict, unsigned int dict_len, unsigned int str_len) {
    // строкой названа последовательность байтов
    if (str_len == 0) {return 1;}
    unsigned int i, j;
    for (i = 256; i < dict_len; ++i) {
        if (dict[i][0] == (str_len + 1) && dict[i][dict[i][0]] == ch) {
            for (j = 0; j <= str_len; ++j) {
                // совпало
                if (j == str_len) {
                    return 1;
                }
                // очевидно различны
                if (str[j] != dict[i][j + 1]) {
                    break;
                }
            }
        }
    }
    return 0;
}

unsigned int get_dict(unsigned char **dict, unsigned int dict_len, unsigned char *str, unsigned int str_len) {
    if (str_len == 1) {return (str[0] + 1);}
    unsigned int i, j;
    for (i = 256; i < dict_len; ++i) {
        if (dict[i][0] == str_len) {
            for (j = 0; j < str_len; ++j) {
                // очевидно различны
                if (str[j] != dict[i][j + 1]) {
                    break;
                }
                // совпало
                if ((j + 1) == str_len) {
                    return (i + 1);
                }
            }
        }
    }
    return 0;
}


// lzw для байтовых последовательностей (сжатие)
// преобразование списка байтов в список некоторых 'чисел'
unsigned int *lzw_encode(unsigned char *input, unsigned long long int size_inp, unsigned long long int *result_len) {
    int mxk = 0; // FOR TESTING
    unsigned int len_key = 0;
    unsigned char *key = malloc(sizeof(char ) * 256); // определение 'строки'-ключа
    // словарь состоит из длины (пока что первый байт) и самой последовательности далее
    unsigned int dict_len = 256; unsigned int dict_mem_step = 8192; unsigned int mem_for_dict = 8192;
    unsigned char **dict = malloc(sizeof(unsigned char *) * mem_for_dict); // определение 'словаря'
    // заполнение словаря всевозможными значениями
    for (int i = 0; i < 256; ++i) { dict[i] = malloc(sizeof(char ) * 2) ; dict[i][0] = 1; dict[i][1] = (char )i;}
    // результат (непонятно сколько памяти держать под каждое значение (типа тип инта))
    unsigned int res_len = 0; unsigned int res_mem_step = 4096; unsigned int mem_for_res = 4096;
    unsigned int *res = malloc(sizeof(unsigned int) * mem_for_res); // сжатая последовательность

    for (unsigned int inp_byte = 0; inp_byte < size_inp; ++inp_byte) { // проход по байтам 'input'
        if (stc_in_dict(key, input[inp_byte], dict, dict_len, len_key) == 1) { // (w+c) in dict
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
            res[res_len] = get_dict(dict, dict_len, key, len_key)-1;
//            printf("%d\n", res[res_len]);
            if (res[res_len] > 65536) {printf("Большая длинна (90 строка)\n"); exit(1);} // хватит два байта
            ++res_len;
            // dict[dict_len] = (key+input[inp_byte])
            if (len_key + 1 <= 255) {
                dict[dict_len] = malloc(sizeof(char ) * (len_key + 2));
                for (unsigned int i = 0; i < len_key; ++i) {dict[dict_len][i+1] = key[i];}
                if (len_key > mxk) { mxk = len_key;} // FOR TESTING
                dict[dict_len][len_key + 1] = input[inp_byte]; dict[dict_len][0] = (unsigned char )(len_key + 1); // типа длинна в нулевом байте
                ++dict_len;
            }
           // key = input[inp_byte]
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
        res[res_len] = get_dict(dict, dict_len, key, len_key)-1;
        ++res_len;
    }
    *result_len = res_len;
    for (unsigned int i = 0; i < dict_len; ++i) {
        free(dict[i]);
    }
    free(dict);
    free(key);
    printf("DEBUG -- mxk: %d\n", mxk);
    return res;
}


char *lzw_decode(unsigned int *input, unsigned long long int size_inp, unsigned long long int *result_len) {
    unsigned int len_key = 0; unsigned int key_mem_step = 4096; unsigned int mem_for_key = 4096;
    unsigned char *key = malloc(sizeof(unsigned char ) * mem_for_key); // определение 'строки'-ключа
    unsigned int len_entry = 0; unsigned int entry_mem_step = 4096; unsigned int mem_for_entry = 4096;
    unsigned char *entry = malloc(sizeof(unsigned char ) * mem_for_entry); // определение entry
    // словарь состоит из длины (пока что первый байт) и самой последовательности далее
    unsigned int dict_len = 256; unsigned int dict_mem_step = 4096; unsigned int mem_for_dict = 4096;
    unsigned char **dict = malloc(sizeof(unsigned char *) * mem_for_dict); // определение 'словаря'
    // заполнение словаря всевозможными значениями
    for (int i = 0; i < 256; ++i) { dict[i] = malloc(sizeof(unsigned char ) * 2) ; dict[i][0] = 1; dict[i][1] = (unsigned char )i;}
    // результат (непонятно сколько памяти держать под каждое значение (типа тип инта))
    unsigned int res_len = 0; unsigned int res_mem_step = 4096; unsigned int mem_for_res = 4096;
    unsigned char *res = malloc(sizeof(unsigned int) * mem_for_res); // сжатая последовательность

    key[0] = (unsigned char )input[0]; len_key = 1;
    res[res_len] = (unsigned char )input[0];
    ++res_len;
    for (unsigned int inp_int = 1; inp_int < size_inp; ++inp_int) { // проход по интам 'input'}
        if (input[inp_int] < dict_len) { // input[inp_int] in dict
            if ((dict[input[inp_int]][0]) >= mem_for_entry) { // в entry памяти не хватит
                while ((dict[input[inp_int]][0]) >= mem_for_entry) {
                    mem_for_entry = mem_for_entry + entry_mem_step;
                }
                entry = realloc(entry, mem_for_entry * sizeof(unsigned char ));
                if (entry == NULL) {
                    fprintf(stderr, "Memory allocation error\n");
                    exit(1);
                }
            }
            len_entry = dict[input[inp_int]][0];
            for (unsigned int i = 0; i < len_entry; ++i) {entry[i] = dict[input[inp_int]][i+1];}
        } else if (input[inp_int] == dict_len) {
            if ((len_key + 1) >= mem_for_entry) { // в entry памяти не хватит
                mem_for_entry = mem_for_entry + entry_mem_step;
                entry = realloc(entry, mem_for_entry * sizeof(unsigned char ));
                if (entry == NULL) {
                    fprintf(stderr, "Memory allocation error\n");
                    exit(1);
                }
            }
            // entry = key + key[0]
            len_entry = len_key;
            for (unsigned int i = 0; i < len_key; ++i) {entry[i] = key[i];}
            entry[len_entry] = key[0];
            ++len_entry;
        } else {
            printf("bad code");
            return 0;
        }
        // памяти под (res + entry) в res не хватит
        if ((res_len + len_entry) >= mem_for_res) {
            while ((res_len + len_entry) >= mem_for_res) {mem_for_res = mem_for_res + res_mem_step;}
            res = realloc(res, mem_for_res * sizeof(unsigned char ));
            if (res == NULL) {
                fprintf(stderr, "Memory allocation error\n");
                exit(1);
            }
        }
        for (unsigned int i = 0; i < len_entry; ++i) { // res += entry
            res[res_len+i] = entry[i];
        }
        res_len += len_entry;
        // в словаре не хватит места под +1 запись
        if ((dict_len + 1) > mem_for_dict) { // память под словарь кончилась
            mem_for_dict = mem_for_dict + dict_mem_step;
            dict = (unsigned char **) realloc(dict, mem_for_dict * sizeof(unsigned char *));
            if (dict == NULL) {
                fprintf(stderr, "Memory allocation error\n");
                exit(1);
            }
        }
//        dictionary[dict_size] = w + entry[0]
        dict[dict_len] = malloc(sizeof(unsigned char ) * (len_key + 1 + 1));
        dict[dict_len][0] = (char )(len_key + 1); // как раз слабый момент храния длинны в нулевом байте (1.)
        for (unsigned int i = 0; i < len_key; ++i) {dict[dict_len][i+1] = key[i];}
        dict[dict_len][1+len_key] = entry[0];
        ++dict_len;

        // памяти под entry в key не хватит
        if (len_entry >= mem_for_key) {
            while (len_entry >= mem_for_key) {mem_for_key = mem_for_key + key_mem_step;}
            key = realloc(key, mem_for_key * sizeof(unsigned char ));
            if (key == NULL) {
                fprintf(stderr, "Memory allocation error\n");
                exit(1);
            }
        }
        // key = entry
        for (int i = 0; i < len_entry; ++i) {key[i] = entry[i];}
        len_key = len_entry;
    }
    *result_len = res_len;
    for (unsigned int i = 0; i < dict_len; ++i) {free(dict[i]);}
    free(entry); free(key); free(dict);
    return res;
}


int main() {
    unsigned long long int size;
    unsigned long long int len_encoded, len_decoded;
// --------------------------
    FILE* file = fopen("2638.txt", "r");
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);
    unsigned char *input = malloc(size);
    fread(input, sizeof(char), size, file);
    fclose(file);
// --------------------------
    unsigned int *encoded = lzw_encode(input, size, &len_encoded);
//    return 0;
    printf("Длинна исходной(байт): %llu. Длинна сжатой('чисел'): %llu\n", size, len_encoded);

    unsigned char *decoded = lzw_decode(encoded, len_encoded, &len_decoded);

    printf("Символов: %llu\n", len_decoded);
    printf("Исходная == Декодированная: ");
    int h = 1;
//    if (len_decoded == size) {
    if (1 == 1) {
        for (unsigned int i = 0; i < len_decoded; ++i) {
            if (input[i]!= decoded[i]) {
                printf("\n%d != %d (%d)\n", input[i], decoded[i], i);
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

// 11.02 -- 90!
// 12.02 -- 150! рабочий алгоритм кодирования LZW на вложенном массиве
// 14.02 -- 270! декодирование LZW на вложенном массиве (2638.txt)
//               1. Словарь хранит длинну в первом символе. Реализовать длинну в первых четырех.
//               2. алгоритм на префиксном дереве https://ru.algorithmica.org/cs/string-structures/trie/ https://habr.com/ru/companies/otus/articles/674378/ https://ru.wikipedia.org/wiki/Алгоритм_Лемпеля_—_Зива_—_Велча

// /usr/bin/time -al ./mlza
// (5477177005 инструкций; 1,93 мб; 1.28 с; 2638.txt; Исходно байт: 50656; Выход байт: [14533*2])
