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
    int numOccurrences;
    int* line_occurrence;
    struct registry* next;
} Registry;

struct index
{
    int tableSize;
    int numKeywords;
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
    (*idx)->numKeywords = 0;

    // faz a leitura do arquivo de palavras-chave.
    FILE* file = fopen(key_file, "r");
    if (file != NULL)
    {
        char* str = NULL;
        int num_keywords = 0;

        // conta o número de palavras-chave para calcular um valor de tableSize.
        while (fgets(str, 17, file) != NULL)
            num_keywords++;

        fclose(file);

        // calcula, a partir do nº de palavras-chave, um valor de tableSize 40% maior.
        (*idx)->tableSize = num_keywords * 1.4;
        (*idx)->hash_table = (Registry**)malloc(sizeof(Registry*) * (*idx)->tableSize);

        for (int i = 0; i < (*idx)->tableSize; i++)
            (*idx)->hash_table[i] = NULL;

        // lê novamente o arquivo 'keys_file.txt', desta vez para armazenar as chaves no índice.
        file = fopen(key_file, "r");
        while (fgets(str, 17, file) != NULL)
        {
            // calcula a chave a partir da palavra-chave fornecida.
            int hash_idx = hash_function(str, (*idx)->tableSize);

            Registry* reg = (Registry*)malloc(sizeof(Registry));
            reg->keyword = str;
            reg->numOccurrences = 0;
            reg->line_occurrence = NULL;
            reg->next = NULL;

            // se a posição inicialmente representada por hash_idx estiver vazia, o novo nó é atribuído a ela.
            if ((*idx)->hash_table[hash_idx] == NULL)
                (*idx)->hash_table[hash_idx] = reg;

            else
            {
                Registry* aux = (*idx)->hash_table[hash_idx];
                while (aux->next != NULL)
                {
                    if (strcmp(aux->keyword, str) == 0) break; // evita inserções repetidas de uma mesma keyword.
                    aux = aux->next;
                }
                // liga o novo registro à coleção com o mesmo código hash.
                aux->next = reg;
            }
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
