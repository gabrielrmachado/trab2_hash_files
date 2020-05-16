//
// Created by gabri on 24/04/2020.
//

#define BUFF_SIZE 17
#define TEXT_BUFF_SIZE 8096

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "index.h"

typedef struct registry
{
    char keyword[BUFF_SIZE];
    int numOccurrences;
    int* line_occurrence;
    struct registry* next; // Tratamento de colisões: encadeamento separado (separate chaining).
} Registry;

typedef struct keyword
{
    char keyword[BUFF_SIZE];
    struct keyword* next;
} Keyword;

struct index
{
    int size;
    int numKeywords;
    int numLinesTextFile;
    char* textFile;
    Keyword* keywords;
    Registry** hash_table;
};

static short insert_keyword(Index** idx, const char* keyword)
{
    short found = 0;
    if ((*idx)->keywords == NULL)
    {
        Keyword* newKw = (Keyword*)malloc(sizeof(Keyword));
        strcpy(newKw->keyword, keyword);
        newKw->next = NULL;
        (*idx)->keywords = newKw;
        (*idx)->numKeywords++;
    }
    else
    {
        Keyword* kw = (*idx)->keywords;
        Keyword* prevKw = NULL;

        while (kw != NULL)
        {
            if (strcmp(kw->keyword, keyword) == 0)
            {
                found = 1;
                break;
            }
            prevKw = kw;
            kw = kw->next;
        }

        if (!found)
        {
            Keyword* newKw = (Keyword*)malloc(sizeof(Keyword));
            strcpy(newKw->keyword, keyword);
            prevKw->next = newKw;
            newKw->next = kw;
            (*idx)->numKeywords++;
        }
    }
    return found;
}

static int compare(const void* a, const void* b)
{
    return strcmp(*(const char**)a, *(const char**)b);
}

static int compare_int(const void* a, const void* b)
{
    return (*(int*)a - *(int*)b);
}

static void get_array_keywords(Index* idx, char*** keywords)
{
    Keyword* kw = idx->keywords;
    *keywords = (char**)malloc(idx->numKeywords * sizeof(char*));

    for (int i = 0; i < idx->numKeywords; i++)
    {
        (*keywords)[i] = malloc((BUFF_SIZE) * sizeof(char));
        strcpy((*keywords)[i], kw->keyword);
        kw = kw->next;
    }
    // coloca o vetor contendo as palavras-chave em ordem alfabética.
    qsort(*keywords, idx->numKeywords, sizeof(const char*), compare);
}

static int hash_function(Index* idx, const char* key)
{
    int sum = 0, len = strlen(key);
    for (int i = 0; i < len; i++)
        sum += (int)key[i];

    int hash = sum % idx->size;
    if (hash >= idx->size)
        return hash -= idx->size;
    return hash;
}

int index_createfrom(const char* key_file, const char* text_file, Index** idx)
{
    // inicializa o índice remissivo.
    *idx = (Index*)malloc(sizeof(Index));
    (*idx)->numKeywords = 0;
    (*idx)->numLinesTextFile = 0;
    (*idx)->keywords = NULL;
    (*idx)->textFile = malloc(sizeof(char) * strlen(text_file)); strcpy((*idx)->textFile, text_file);

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
        {
            // remove o '\n' da string.
            char* pos = strchr(str, '\n');
            if (pos != NULL) *pos = '\0';
            insert_keyword(idx, str);
        }

        fclose(file);
        (*idx)->size = (*idx)->numKeywords * 1.5;
        (*idx)->hash_table = (Registry**)malloc(sizeof(Registry*) * (*idx)->size);

        for (int i = 0; i < (*idx)->size; i++)
            (*idx)->hash_table[i] = NULL;

        char** keywords;
        get_array_keywords(*idx, &keywords);

        for (int i = 0; i < (*idx)->numKeywords; i++)
        {
            char str[BUFF_SIZE]; strcpy(str, keywords[i]);

            // calcula a chave a partir da palavra-chave fornecida.
            int hash_idx = hash_function(*idx, str);

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

    Keyword* kw = (*idx)->keywords;
    for (int i = 0; i < (*idx)->numKeywords; i++)
    {
        index_put(*idx, kw->keyword);
        kw = kw->next;
    }

    return 0;
}
int index_get(const Index* idx, const char* key, int** occurrences, int* num_ocurrences)
{
    int hash = hash_function(idx, key);
    Registry* reg = idx->hash_table[hash];

    while (reg != NULL && strcmp(reg->keyword, key) != 0)
        reg = reg->next;

    if (reg == NULL) return 1;
    else
    {
        *num_ocurrences = reg->numOccurrences;
        *occurrences = (int*)malloc(sizeof(int) * idx->numLinesTextFile);
        memset(*occurrences, -1, sizeof(int) * idx->numLinesTextFile);

        // copia o vetor de ocorrência contido no índice para o parâmetro 'occurrences'.
        int j = 0;
        for (int i = 0; i < idx->numLinesTextFile; i++)
        {
            if (reg->line_occurrence[i] != -1)
                (*occurrences)[j++] = reg->line_occurrence[i];
        }

        return 0;
    }
}
int index_put(const Index* idx, const char* key)
{
    // com as keywords inseridas, agora é hora de ler o texto contido em 'text_file.txt'.
    FILE* file = fopen(idx->textFile, "r");
    rewind(file); int ans = -1;

    if (file != NULL)
    {
        int current_line = 0;
        int num_occurrences = 0;
        int* occurrences = (int*)malloc(sizeof(int) * idx->numLinesTextFile);
        memset(occurrences, -1, sizeof(int) * idx->numLinesTextFile);

        char str[TEXT_BUFF_SIZE] = "\0";
        while (fgets(str, TEXT_BUFF_SIZE, file) != NULL)
        {
            current_line++;

            // separa as palavras da frase capturada.
            int i = 0;
            while (i < strlen(str))
            {
                char word[BUFF_SIZE] = "\0"; int j = 0;
                if (!isalpha(str[i])) { i++; continue; }

                while (isalpha(str[i]))
                    word[j++] = str[i++];

                if (strcmp(word, key) == 0) // encontrou keyword no texto.
                {
                    num_occurrences++;

                    // assegura que a atual linha de texto não seja repetida várias vezes em 'occurrences'.
                    if (occurrences[current_line-1] == -1)
                        occurrences[current_line-1] = current_line;
                }
            }
        }

        if (num_occurrences > 0)
        {
            // procura a keyword no índice remissivo.
            int hash = hash_function(idx, key);
            Registry* reg = idx->hash_table[hash];
            Registry* antReg = reg;

            while (reg != NULL && strcmp(reg->keyword, key) != 0)
            {
                antReg = reg;
                reg = reg->next;
            }

            if (reg == NULL) // não encontrou a keyword.
            {
                Registry* newReg = (Registry*) malloc(sizeof(Registry));
                strcpy(newReg->keyword, key);
                newReg->numOccurrences = num_occurrences;
                newReg->line_occurrence = (int*)malloc(sizeof(int) * idx->numLinesTextFile);
                memset(newReg->line_occurrence, 0, sizeof(int));

                for (int i = 0; i < idx->numLinesTextFile; i++)
                    newReg->line_occurrence[i] = occurrences[i];

                // primeiro registro no slot.
                if (antReg == NULL)
                    idx->hash_table[hash] = newReg;

                else antReg->next = newReg;
                newReg->next = NULL;

                // atualiza o vetor de keywords.
                insert_keyword(&idx, key);
            }
            else
            {
                reg->numOccurrences = num_occurrences;
                for (int i = 0; i < idx->numLinesTextFile; i++)
                    reg->line_occurrence[i] = occurrences[i];
            }

            free(occurrences);
        }
        ans = 0;
    }
    else
    {
        fprintf(stderr, "\nFile %s not found.", idx->textFile);
        ans = 1;
    }
    fclose(file);
    return ans;
}

int index_print(const Index* idx)
{
    char** keywords;
    get_array_keywords(idx, &keywords);

    // a partir do vetor de keywords ordenado, são feitas as consultas na tabela hash.
    for (int i = 0; i < idx->numKeywords; i++)
    {
        char keyword[BUFF_SIZE]; strcpy(keyword, keywords[i]);
        int* occurrences; int num_occurrences;
        printf("%s: ", keyword);

        index_get(idx, keyword, &occurrences, &num_occurrences);
        qsort(occurrences, idx->numLinesTextFile, sizeof(int), compare_int);

        for (int j = 0; j < idx->numLinesTextFile; j++)
        {
            if (occurrences[j] != -1)
            {
                if (j < idx->numLinesTextFile-1) printf("%d, ", occurrences[j]);
                else printf("%d\n", occurrences[j]);
            }
        }
    }
    return 0;
}