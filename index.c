//
// Created by gabri on 24/04/2020.
//

#define BUFF_SIZE 17
#define TEXT_BUFF_SIZE 8096

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "index.h"

typedef struct registry
{
    char keyword[BUFF_SIZE];
    int numOccurrences;
    int* line_occurrence;
    struct registry* next;
} Registry;

struct index
{
    int numKeywords;
    char* keywords[BUFF_SIZE];
    Registry** hash_table;
};

static int hash_function(char* key, int TABLE_SIZE)
{
    int sum = 0, len = strlen(key);

    for (int i = 0; i < len; i++)
        sum += (int)key[i];

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
        char str[BUFF_SIZE] = "\0";

        // conta o número de palavras-chave para calcular um valor de tableSize.
        while (fgets(str, BUFF_SIZE, file) != NULL)
            (*idx)->numKeywords++;

        fclose(file);

        (*idx)->hash_table = (Registry**)malloc(sizeof(Registry*) * (*idx)->numKeywords);

        for (int i = 0; i < (*idx)->numKeywords; i++)
            (*idx)->hash_table[i] = NULL;

        // lê novamente o arquivo 'keys_file.txt', desta vez para armazenar as chaves no índice.
        file = fopen(key_file, "r");
        rewind(file);
        int i = 0;

        while (fgets(str, BUFF_SIZE, file) != NULL)
        {
            // remove o '\n' da string.
            char* pos = strchr(str, '\n');
            if (pos != NULL) *pos = '\0';

            (*idx)->keywords[i] = malloc(strlen(str) + 1);
            strcpy((*idx)->keywords[i], str); i++;

            // calcula a chave a partir da palavra-chave fornecida.
            int hash_idx = hash_function(str, (*idx)->numKeywords);

            Registry* reg = (Registry*)malloc(sizeof(Registry));
            strcpy(reg->keyword, str);
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
        fclose(file);
    }
    else
    {
        fprintf(stderr, "\nFile %s not found.", key_file);
        return 1;
    }

    // com as keywords inseridas, agora é hora de ler o texto contido em 'text_file.txt'.
    file = fopen(text_file, "r");
    if (file != NULL)
    {
        char str[TEXT_BUFF_SIZE] = "\0";
        while (fgets(str, TEXT_BUFF_SIZE, file) != NULL)
        {
            // separa as palavras da frase capturada.
            int i = 0;
            while (i < strlen(str))
            {
                char word[BUFF_SIZE] = "\0";
                int j = 0;

                // verifica se o caractere atual é uma letra. Se não for, o mesmo é ignorado.
                if (!isalpha(str[i])) i++;

                // Se for, executa um loop para capturar as demais letras que formam a palavra.
                // Esse loop é executado até que um caractere não alfabético surja.
                else while (isalpha(str[i]) && i < strlen(str) && j < BUFF_SIZE-1)
                {
                    word[j] = str[i];
                    i++; j++;
                }
            }
        }
    }
    else
    {
        fprintf(stderr, "\nFile %s not found.", text_file);
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
