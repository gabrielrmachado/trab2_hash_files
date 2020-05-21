//
// Created by gabri on 24/04/2020.
//

#define BUFF_SIZE 17

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "index.h"

typedef struct occurrence
{
    int num_occurrence;
    int line;
} Occurrence;

typedef struct registry
{
    char keyword[BUFF_SIZE];
    int numOccurrences;
    Occurrence** line_occurrence;
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

    return sum % idx->size;
}

int index_createfrom(const char* key_file, const char* text_file, Index** idx)
{
    // inicializa o índice remissivo.
    *idx = (Index*)malloc(sizeof(Index));
    (*idx)->numKeywords = 0;
    (*idx)->numLinesTextFile = 1;
    (*idx)->keywords = NULL;
    (*idx)->textFile = malloc(sizeof(char) * strlen(text_file)); strcpy((*idx)->textFile, text_file);

    // conta o número de linhas de texto em 'text_file.txt'.
    FILE* file = fopen(text_file, "r");

    if (file != NULL)
    {
        do
        {
            if (feof(file)) break;

            int c = fgetc(file);
            if (c == '\n') (*idx)->numLinesTextFile++;
        }
        while (1);
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
        (*idx)->size = (*idx)->numKeywords * 1.5; // MUDAR ISSO...
        (*idx)->hash_table = (Registry**)malloc(sizeof(Registry*) * (*idx)->size);
        memset((*idx)->hash_table, 0, sizeof(Registry*) * (*idx)->size);

        char** keywords;
        get_array_keywords(*idx, &keywords);

//        for (int i = 0; i < (*idx)->numKeywords; i++)
//        {
//            char str[BUFF_SIZE]; strcpy(str, keywords[i]);
//
//            // calcula a chave a partir da palavra-chave fornecida.
//            int hash_idx = hash_function(*idx, str);
//
//            Registry* reg = (Registry*)malloc(sizeof(Registry));
//            strcpy(reg->keyword, str);
//            reg->numOccurrences = 0;
//            reg->line_occurrence = (Occurrence**)malloc(sizeof(Occurrence*) * (*idx)->numLinesTextFile);
//            memset(reg->line_occurrence, 0, sizeof(Occurrence*) * (*idx)->numLinesTextFile);
//            reg->next = NULL;
//
//            // se a posição inicialmente representada por hash_idx estiver vazia, o novo nó é atribuído a ela.
//            if ((*idx)->hash_table[hash_idx] == NULL)
//                (*idx)->hash_table[hash_idx] = reg;
//
//            else
//            {
//                Registry* aux = (*idx)->hash_table[hash_idx];
//                while (aux->next != NULL)
//                {
//                    if (strcmp(aux->keyword, str) == 0) break; // evita inserções repetidas de uma mesma keyword.
//                    aux = aux->next;
//                }
//                // liga o novo registro à coleção com o mesmo código hash.
//                aux->next = reg;
//            }
//        }
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
        // verifica as ocorrências de cada palavra-chave no texto.
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
        *occurrences = (int*)malloc(sizeof(int) * reg->numOccurrences);

        // copia o vetor de ocorrência contido no índice para o parâmetro 'occurrences'.
        int i = 0, j = 0;
        while (i < idx->numLinesTextFile)
        {
            Occurrence* occ = reg->line_occurrence[i++]; int k = 0;
            if (occ != 0)
            {
                while (k < occ->num_occurrence)
                {
                    (*occurrences)[j++] = occ->line;
                    k++;
                }
            }
        }
        return 0;
    }
}
int index_put(const Index* idx, const char* key)
{
    FILE* file = fopen(idx->textFile, "r");
    rewind(file); int ans = -1;

    if (file != NULL)
    {
        int current_line = 1, i = 0;
        int num_occurrences = 0;
        Occurrence** occurrences = (Occurrence**)malloc(sizeof(Occurrence*) * idx->numLinesTextFile);
        memset(occurrences, 0, sizeof(Occurrence*) * idx->numLinesTextFile);
        char word[BUFF_SIZE] = {"\0"};

        do
        {
            if (feof(file)) break;
            int c = fgetc(file);

            // separa as palavras da frase capturada.
            if (c == '\n' || c == '\t' || c == ' ')
            {
                if (strcmp(word, "") == 0)
                {
                    if (c == '\n') current_line++;
                    continue;
                }
                if (c == '\n') current_line++;
                memset(word, '\0', sizeof(char) * BUFF_SIZE); i = 0;
            }
            else
            {
                // verifica palavras maiores que 16 caracteres. (TESTAR ISSO...)
                if (i >= BUFF_SIZE) continue;
                word[i++] = c;

                if (strcmp(word, key) == 0)
                {
                    num_occurrences++;
                    Occurrence* occ = occurrences[current_line - 1];

                    // assegura que a atual linha de texto não seja repetida várias vezes em 'occurrences'.
                    if (occ == 0)
                    {
                        occ = (Occurrence*)malloc(sizeof(Occurrence));
                        occ->num_occurrence = 1;
                        occ->line = current_line;
                        occurrences[current_line - 1] = occ;
                    }
                    else
                    {
                        occ->num_occurrence++;
                    }
                }
            }
        }
        while (1);

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
                newReg->line_occurrence = (Occurrence**)malloc(sizeof(Occurrence*) * idx->numLinesTextFile);
                memset(newReg->line_occurrence, 0, sizeof(Occurrence*) * idx->numLinesTextFile);
                newReg->next = NULL;

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
        else
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

            if (reg != NULL)
            {
                reg->numOccurrences = 0;
                memset(reg->line_occurrence, 0, sizeof(Occurrence*) * idx->numLinesTextFile);
            }
            else
            {
                Registry* newReg = (Registry*)malloc(sizeof(Registry));
                strcpy(newReg->keyword, key);
                newReg->numOccurrences = 0;
                newReg->line_occurrence = (Occurrence**)malloc(sizeof(Occurrence*) * idx->numLinesTextFile);
                memset(newReg->line_occurrence, 0, sizeof(Occurrence*) * idx->numLinesTextFile);
                newReg->next = NULL;

                if (antReg == NULL)
                    idx->hash_table[hash] = newReg;

                else antReg->next = newReg;
            }
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

        index_get(idx, keyword, &occurrences, &num_occurrences);

        if (num_occurrences > 0)
        {
            printf("%s: ", keyword);
            for (int j = 0; j < num_occurrences; j++)
            {
                if (j < num_occurrences-1) printf("%d, ", occurrences[j]);
                else printf("%d\n", occurrences[j]);
            }
        }
    }
    return 0;
}