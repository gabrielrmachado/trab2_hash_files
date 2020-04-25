//
// Created by gabri on 24/04/2020.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "index.h"

static int TABLE_SIZE = 0;

typedef struct registry
{
    char* keyword;
    int* line_occurrence;
    struct registry* next;
} Registry;

struct index
{
    int* hash_keys;  // armazena as chaves hash calculadas para cada keyword.
    Registry* hash_table;
};

static int hash_function(char* key)
{
    srand(time(NULL));
    int sum = 0, len = strlen(key);

    for (int i = 0; i < len; i++)
        sum += (int)key[i] * (rand() % len-1);

    return sum % TABLE_SIZE;
}

int index_createfrom(const char* key_file, const char* text_file, Index** idx)
{
    // inicializa o índice remissivo.
    *idx = (Index*)malloc(sizeof(Index));

    // faz a leitura do arquivo de palavras-chave.
    FILE* file = fopen(key_file, "r");
    if (file != NULL)
    {
        char* str = NULL;
        int num_keywords = 0;

        // conta o número de palavras-chave para calcular um valor de TABLE_SIZE.
        while (fgets(str, 17, file) != NULL)
            num_keywords++;

        fclose(file);

        // calcula, a partir do nº de palavras-chave, um valor de TABLE_SIZE 50% maior.
        TABLE_SIZE = num_keywords * 1.5;
        (*idx)->hash_table = (Registry*)malloc(sizeof(Registry) * TABLE_SIZE);

        // lê novamente o arquivo 'keys_file.txt', desta vez para armazenar as chaves no índice.
        file = fopen(key_file, "r");
        while (fgets(str, 17, file) != NULL)
        {
            int hash_idx = hash_function(str);

        }
    }
    else
    {
        fprintf(stderr, "\nFile %s not found.", key_file);
        return 1;
    }

    return 0;
}
int index_get(const Index* idx, const char* key, int** occurrences, int* num_ocurrences)
{
    return 0;
}
int index_put(const Index* idx, const char* key)
{
    return 0;
}
int index_print(const Index* idx)
{
    return 0;
}
