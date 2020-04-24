//
// Created by gabri on 24/04/2020.
//

#include <stdlib.h>
#include <stdio.h>
#include "index.h"

static int TABLE_SIZE = 0;

typedef struct registry
{
    char* key;
    int line_occurrence;
    struct registry* next;
} Registry;

struct index
{
    Registry* hash_table;
};

static int hash_function(char* key)
{
    return 0;
}

int index_createfrom(const char* key_file, const char* text_file, Index** idx)
{
    // inicializa o índice.
    *idx = (Index*)malloc(sizeof(Index));
    (*idx)->hash_table = (Registry*)malloc(sizeof(Registry) * TABLE_SIZE);

    // faz a leitura do arquivo de palavras-chave.
    FILE* file = fopen(key_file, "r");
    if (file != NULL)
    {
        char* str;
        while (fgets(str, 17, file) != NULL)
        {
            printf("%s", str);
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
