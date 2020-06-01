#define BUFF_SIZE 17

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "index.h"

typedef struct occurrence // struct para armazenar a qtde de repetições de uma keyword em uma mesma linha de texto.
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

typedef struct keyword // estrutura dos nós pertencentes à lista encadeada de keywords.
{
    char keyword[BUFF_SIZE];
    struct keyword* next;
} Keyword;

struct index
{
    int size; // armazena o tamanho da tabela hash, i.e., o número de slots máximo do array.
    int numKeywords; // armazena o número de keywords.
    int numLinesTextFile; // armazena o número de linhas do arquivo de texto.
    char* textFile; // armazena a path string do arquivo de texto.
    Keyword* keywords; // lista encadeada em tempo de execução. Útil para realizar o sorting mais rapidamente.
    Registry** hash_table; // a tabela hash.
};

static char *ltrim(char *s)
{
    while(isspace(*s)) s++;
    return s;
}

static char *rtrim(char *s)
{
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}

static char *trim(char *s)
{
    return rtrim(ltrim(s));
}

static short insert_keyword(Index** idx, const char* keyword)
// insere, em tempo de execução, uma nova keyword na lista encadeada '(*idx)->keywords'.
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

static short isPrime(int N)
{
    if (N % 2 == 0) return 0;
    else
    {
        int half = N / 2, i = 2;
        while (i < half)
        {
            if (N % i == 0) return 0;
            i++;
        }
        return 1;
    }
}

static int compute_size_array(const int numKeywords)
// calcula um número para o tamanho para a tabela hash que seja primo, de modo a tornar as colisões menos frequentes.
{
    int pSize = 0;

    if (numKeywords <= 20) pSize = numKeywords * 2;
    else pSize = numKeywords * 1.4;

    while (!isPrime(pSize))
        pSize++;

    return pSize;
}

static int compare(const void* a, const void* b)
{
    return strcmp(*(const char**)a, *(const char**)b);
}

static void get_array_keywords(Index* idx, char*** keywords)
// retorna no array 'keywords' todas as keywords, armazenadas na lista encadeada, em ordem lexicográfica.
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
// a função hash que calcula o índice baseado no método da divisão.
{
    int sum = 0, len = strlen(key);
    for (int i = 0; i < len; i++)
        sum += (int)key[i];

    return sum % idx->size;
}

int index_createfrom(const char* key_file, const char* text_file, Index** idx)
{
    if (*idx != NULL)
    {
        printf("Pointer 'idx' is not NULL!\n");
        return 1;
    }

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

        // conta o número de palavras-chave para calcular um tamanho para a tabela hash.
        while (fgets(str, BUFF_SIZE, file) != NULL)
        {
            // remove o '\n' da string.
            char* pos = strchr(str, '\n');
            if (pos != NULL) *pos = '\0';
            insert_keyword(idx, strlwr(trim(str)));
        }

        fclose(file);
        (*idx)->size = compute_size_array((*idx)->numKeywords);
        (*idx)->hash_table = (Registry**)malloc(sizeof(Registry*) * (*idx)->size);
        memset((*idx)->hash_table, 0, sizeof(Registry*) * (*idx)->size);

        char** keywords;
        get_array_keywords(*idx, &keywords);
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
    if (idx == NULL) { printf("Null pointer!\n"); return 1; }

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
    if (idx == NULL) { printf("Null pointer!\n"); return 1; }

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
                //strstr
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
                // verifica palavras maiores que 16 caracteres.
                if (i >= BUFF_SIZE) continue;
                word[i++] = tolower(c);

                if (strcmp(word, key) == 0)
                {
                    char c_next = fgetc(file);
                    ungetc(c_next, file);

                    if (!isalpha(c_next))
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

    if (idx == NULL) { printf("Null pointer.\n");  return 1; }

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