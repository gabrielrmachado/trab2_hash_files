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
    struct registry* next; // Tratamento de colisões: encadeamento separado (separate chaining).
} Registry;

struct index
{
    int numKeywords;
    int numLinesTextFile;
    char* keywords[BUFF_SIZE];
    Registry** hash_table;
};

static int compare(const void* a, const void* b)
{
    return strcmp(*(const char**)a, *(const char**)b);
}

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
    (*idx)->numLinesTextFile = 0;

    // conta o número de linhas de texto em 'text_file.txt'.
    FILE* file = fopen(text_file, "r");

    if (file != NULL)
    {
        char str[TEXT_BUFF_SIZE] = "\0";
        while (fgets(str, TEXT_BUFF_SIZE, file) != NULL)
        {
            (*idx)->numLinesTextFile++;
        }
    }
    fclose(file);

    // faz a leitura do arquivo de palavras-chave.
    file = fopen(key_file, "r");
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
            reg->line_occurrence = (int*)malloc(sizeof(int) * (*idx)->numLinesTextFile);
            memset(reg->line_occurrence, 0, sizeof(int));
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
    rewind(file);

    if (file != NULL)
    {
        int current_line = 0;
        char str[TEXT_BUFF_SIZE] = "\0";
        while (fgets(str, TEXT_BUFF_SIZE, file) != NULL)
        {
            current_line++;

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
                else
                {
                    while (isalpha(str[i]) && i < strlen(str) && j < BUFF_SIZE-1)
                    {
                        word[j] = str[i];
                        i++; j++;
                    }

                    // verifica se a keyword já existe no índice.
                    int h_idx = hash_function(word, (*idx)->numKeywords);
                    Registry* reg = (*idx)->hash_table[h_idx];

                    while (reg != NULL)
                    {
                        if (strcmp(word, reg->keyword) == 0)
                        {
                            // encontrou a keyword.
                            reg->line_occurrence[reg->numOccurrences++] = current_line;
                            break;
                        }
                        reg = reg->next;
                    }
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
    int hash = hash_function(key, idx->numKeywords);
    bool found = false;

    Registry* reg = idx->hash_table[hash];

    while (strcmp(reg->keyword, key) != 0 && reg != NULL)
        reg = reg->next;

    if (reg == NULL) return 1;
    else
    {
        *num_ocurrences = reg->numOccurrences;
        *occurrences = (int*)malloc(sizeof(int) * (*num_ocurrences));

        // copia o vetor de ocorrência contido no índice para o parâmetro 'occurrences'.
        for (int i = 0; i < (*num_ocurrences); i++)
            (*occurrences)[i] = reg->line_occurrence[i];

        return 0;
    }
}
int index_put(const Index* idx, const char* key)
{
    return 0;
}
int index_print(const Index* idx)
{
    // coloca o vetor contendo as palavras-chave em ordem alfabética.
    qsort(idx->keywords, idx->numKeywords, sizeof(const char*), compare);

    // a partir do vetor de keywords ordenado, são feitas as consultas na tabela hash.
    for (int i = 0; i < idx->numKeywords; i++)
    {
        char keyword[BUFF_SIZE]; strcpy(keyword, idx->keywords[i]);
        int* occurrences; int num_occurrences;
        printf("%s: ", keyword);

        index_get(idx, keyword, &occurrences, &num_occurrences);

        for (int j = 0; j < num_occurrences-1; j++)
            printf("%d, ", occurrences[j]);

        printf("%d\n", occurrences[num_occurrences-1]);

    }
    return 0;
}
