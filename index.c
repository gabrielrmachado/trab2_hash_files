//
// Created by gabri on 24/04/2020.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "index.h"

typedef struct registry
{
    char* keyword;
    int* line_occurrence;
    struct registry* next;
} Registry;

struct index
{
    int TABLE_SIZE;
    Registry** hash_table;
};

static int hash_function(char* key, int TABLE_SIZE)
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

        // calcula, a partir do nº de palavras-chave, um valor de TABLE_SIZE 40% maior.
        (*idx)->TABLE_SIZE = num_keywords * 1.4;
        (*idx)->hash_table = (Registry**)malloc(sizeof(Registry*) * (*idx)->TABLE_SIZE);

        for (int i = 0; i < (*idx)->TABLE_SIZE; i++)
            (*idx)->hash_table[i] = NULL;

        // lê novamente o arquivo 'keys_file.txt', desta vez para armazenar as chaves no índice.
        file = fopen(key_file, "r");
        while (fgets(str, 17, file) != NULL)
        {
            // calcula a chave a partir da palavra-chave fornecida.
            int hash_idx = hash_function(str, (*idx)->TABLE_SIZE);
            Registry* reg_prev = (*idx)->hash_table[hash_idx];
            Registry* reg_next = (*idx)->hash_table[hash_idx];
            bool found = false;

            // verifica se o slot correspondente à hash_key está vazio.
//            while (reg->next != NULL)
//            {
//                // a palavra-chave já existe.
//                if (!strcmp(reg->keyword, str))
//                {
//                    found = true;
//                    break;
//                }
//            }
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
