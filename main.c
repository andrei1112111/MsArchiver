#include <stdio.h>
#include <stdlib.h>


// сжатие реализованно при помощи префиксного дерева
// vertex_id - индекс в массиве ('порядковый номер' фразы в списке всех фраз)
// pVertex - список возможных 'путей'
struct Vertex {
    unsigned long long int vertex_id;
    struct Vertex *pVertex[256];
};

// добавление 'фразы' в дерево
void add_vertex(struct Vertex *root, struct Vertex *arrVertex, unsigned long long int *lVertex, const unsigned char *key , unsigned int len_key) {
    unsigned int i;
    struct Vertex *cur = root;
    for (i = 0; i < len_key; i++) {
        if (cur->pVertex[key[i]] == NULL) {
            arrVertex[*lVertex].vertex_id = *lVertex;
            for (int j = 0; j < 256; ++j) {arrVertex[i].pVertex[j] = NULL;}
            cur->pVertex[key[i]] = &arrVertex[*lVertex];         // ОШИБКА В ТОМ ЧТО ПРИ РЕЛОВКАЦИИ АДРЕСА СЛЕТАЮТ ЧЕ С ЭТИМ ДЕЛАТЬ А
            *lVertex = *lVertex + 1;
        }
        cur = cur->pVertex[key[i]];
    }
}

// поиск 'фразы' в дереве
// возвращает индекс в массиве ('порядковый номер' фразы в списке всех фраз) (фразы не нашлось - (0), нашлась - (номер + 1))
unsigned int find_vertex(struct Vertex *root, const unsigned char *key, unsigned long long int len_key) {
    unsigned char j;
    struct Vertex *cur = root;
    for (unsigned int i = 0; i < len_key; ++i) {
        j = (unsigned char)key[i];
        cur = cur->pVertex[j];
        if (cur == NULL) {
            return 0;
        }
    }
    return (cur->vertex_id + 1);
}

// lzw для байтовых последовательностей (сжатие при помощи префиксного дерева)
// максимальный размер входных данных - (18 446 744 073 709 551 615 байт) == (16,7 терабайта)
// возвращает массив 'чисел' - кодов всех (без потерь) фраз
unsigned long long int *lzw_encode(const unsigned char *input, unsigned long long int size_inp, unsigned long long int *result_len) {
    // определение строки-фразы
    unsigned long long int len_key = 0;
    unsigned long long int key_mem_step = 256; unsigned long long int mem_for_key = 256;
    unsigned char *key = malloc(sizeof(unsigned char ) * mem_for_key);
    // определение дерева для хранения фраз
    unsigned long long int lVertex = 256;
    unsigned long long int vert_mem_step = 8192; unsigned long long int mem_for_vert = 8192;
    struct Vertex root;
    struct Vertex *arrVertex = malloc(sizeof(struct Vertex) * mem_for_vert);
    // предзаполнение корня всеми возможными символами (диапазон char)
    for (int j = 0; j < 256; ++j) {root.pVertex[j] = NULL;}
    for (int i = 0; i < 256; ++i) {
        arrVertex[i].vertex_id = i;
        for (int j = 0; j < 256; ++j) {arrVertex[i].pVertex[j] = NULL;} // символ = номер символа в pVertex
        root.pVertex[i] = &arrVertex[i];
    }
    // результат (жрет много памяти)       // !WORK! Возможно стоит сделать условие выбора разных типов, если предполагать что res_len <= in
    unsigned long long int res_len = 0;
    unsigned long long int res_mem_step = 4096; unsigned long long int mem_for_res = 4096;
    unsigned long long int *res = malloc(sizeof(unsigned long long int) * mem_for_res); // коды фраз, результат работы функции lzw_encode

    for (unsigned int inp_byte = 0; inp_byte < size_inp; ++inp_byte) { // проход по байтам 'input'
        if ((len_key + 1) >= mem_for_key) { // в key памяти не хватит под добавление нового символа
            mem_for_key = mem_for_key + key_mem_step;
            key = realloc(key, mem_for_key * sizeof(unsigned char ));
            if (key == NULL) {
                fprintf(stderr, "Memory allocation error at ->encoding ->for ->key\n");
                free(arrVertex); free(key); free(res);
                exit(1);
            }
        }
        key[len_key] = input[inp_byte]; ++len_key; // [key] = key+input[inp_byte]
        if (find_vertex(&root, key, len_key) == 0) { // проверка (key+input[inp_byte]) не в dict
            if ((lVertex + 1) > mem_for_vert) { // память под дерево кончилась
                mem_for_vert = mem_for_vert + vert_mem_step;
                arrVertex = (struct Vertex *) realloc(arrVertex, mem_for_vert * sizeof(struct Vertex));
                if (arrVertex == NULL) {
                    fprintf(stderr, "Memory allocation error at ->encoding ->for ->arrVertex\n");
                    free(arrVertex); free(key); free(res);
                    exit(1);
                }
            }
            if ((res_len + 1) > mem_for_res) { // память под результат кончилась
                mem_for_res = mem_for_res + res_mem_step;
                res = realloc(res, mem_for_res * sizeof(unsigned long long int));
                if (res == NULL) {
                    fprintf(stderr, "Memory allocation error at ->encoding ->for ->res\n");
                    free(arrVertex); free(key); free(res);
                    exit(1);
                }
            }
            // result.append(dictionary[key])
            res[res_len] = find_vertex(&root, key, (len_key-1))-1;
            ++res_len;
            // dictionary.append(key+input[inp_byte])
            add_vertex(&root, arrVertex, &lVertex, key, len_key);
           // key = input[inp_byte]
            key[0] = input[inp_byte]; len_key = 1;
        }
    }
    if (len_key != 0) { // key не пустой
        if ((res_len + 1) > mem_for_res) { // память под результат кончилась
            mem_for_res = mem_for_res + res_mem_step;
            res = realloc(res, mem_for_res * sizeof(unsigned long long int));
            if (res == NULL) {
                fprintf(stderr, "Memory allocation error at ->encoding ->main ->res\n");
                free(arrVertex); free(key); free(res);
                exit(1);
            }
        }
        // result.append(dictionary[key])
        res[res_len] = (find_vertex(&root, key, len_key)-1);
        ++res_len;
    }
    *result_len = res_len;
    free(arrVertex); free(key);
    return res;
}


// lzw для байтовых последовательностей (Декодирование при помощи Массива строк)
// принимает последовательность чисел-кодов и возвращает исходную последовательность (без потерь)
char *lzw_decode(const unsigned long long int *input, unsigned long long int size_inp, unsigned long long int *result_len) {
    unsigned long long int len_key = 0; unsigned long long int key_mem_step = 4096; unsigned long long int mem_for_key = 4096;
    unsigned char *key = malloc(sizeof(unsigned char ) * mem_for_key); // определение 'строки'-ключа
    unsigned long long int len_entry = 0; unsigned long long int entry_mem_step = 4096; unsigned long long int mem_for_entry = 4096;
    unsigned char *entry = malloc(sizeof(unsigned char ) * mem_for_entry); // определение entry
    // словарь состоит из длины (пока что первый байт) и самой последовательности далее
    unsigned long long int dict_len = 256; unsigned long long int dict_mem_step = 4096; unsigned long long int mem_for_dict = 4096;
    unsigned char **dict = malloc(sizeof(unsigned char *) * mem_for_dict); // определение 'словаря'
    // заполнение словаря всевозможными значениями
    for (int i = 0; i < 256; ++i) { dict[i] = malloc(sizeof(unsigned char ) * 2) ; dict[i][0] = 1; dict[i][1] = (unsigned char )i;}
    // результат (непонятно сколько памяти держать под каждое значение (типа тип инта))
    unsigned long long int res_len = 0; unsigned long long int res_mem_step = 4096; unsigned long long int mem_for_res = 4096;
    unsigned char *res = malloc(sizeof(unsigned char ) * mem_for_res); // сжатая последовательность

    key[0] = (unsigned char )input[0]; len_key = 1;
    res[res_len] = (unsigned char )input[0];
    ++res_len;
    for (unsigned long long int inp_int = 1; inp_int < size_inp; ++inp_int) { // проход по интам 'input'}
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
            for (unsigned long long int i = 0; i < len_entry; ++i) {entry[i] = dict[input[inp_int]][i+1];}
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
            for (unsigned long long int i = 0; i < len_key; ++i) {entry[i] = key[i];}
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
        for (unsigned long long int i = 0; i < len_entry; ++i) { // res += entry
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
        for (unsigned long long int i = 0; i < len_key; ++i) {dict[dict_len][i+1] = key[i];}
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
    for (unsigned long long int i = 0; i < dict_len; ++i) {free(dict[i]);}
    free(entry); free(key); free(dict);
    return res;
}


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

    printf("Длинна исходной(байт): %llu. Длинна сжатой('чисел'): %llu\n", size, len_encoded);

    unsigned char *decoded = lzw_decode(encoded, len_encoded, &len_decoded);

//    for (unsigned int i = 0; i < len_encoded; ++i) { printf("%llu ", encoded[i]); } printf("\n");
//    for (unsigned int i = 0; i < len_decoded; ++i) { printf("%c", decoded[i]); } printf("\n");

    printf("Символов: %llu\n", len_decoded);
    printf("Исходная == Декодированная: ");
    int h = 1;
    if (len_decoded == size) {
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
// 15ю02 -- 270! префиксное дерево

//              1. Словарь хранит длинну в первом символе. Реализовать длинну в первых четырех.


// /usr/bin/time -al ./mlza
