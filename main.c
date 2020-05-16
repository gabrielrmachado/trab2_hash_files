#include <stdio.h>
#include <stdlib.h>
#include "index.h"

int program1()
{
    Index* idx;
    char* argv1 = "keys_file.txt";
    char* argv2 = "text_file.txt";
    index_createfrom(argv1, argv2, &idx);
    index_print(idx);
    index_put(idx, "read"); printf("\n");
    index_print(idx);
    index_put(idx, "the"); printf("\n");
    index_print(idx);
    index_put(idx, "of"); printf("\n");
    index_print(idx);
    index_put(idx, "programming"); printf("\n");
    index_print(idx);
    return 0;
}

int program2(int argc, char** argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Error: insufficient number of parameters:\nSintax: %s " "key_file_name txt_file_name\n", argv[0]);
        return 1;
    }

    Index* idx;
    if (index_createfrom(argv[1], argv[2], &idx))
    {
        fprintf(stderr, "Error when creating index.\n ");
        return 1;
    }

    char keyword[17];
    printf("Which keyword do you wish to look for?\n");
    scanf(" %16[^\n]", keyword);

    int* occurrences; int n_occurrences;
    if (index_get(idx, keyword, &occurrences, &n_occurrences))
        fprintf(stderr, "Error: the keyword %s does not belong to the index.\n", keyword);
    else
    {
        if (n_occurrences <= 0)
            printf("There is no occurrence of the keyword %s\n", keyword);
        else
        {
            printf("%d occurrences of %s\n", n_occurrences, keyword);
            for (int i = 0; i < n_occurrences-1; i++)
                printf("%d, ", occurrences[i]);
            printf("%d\n", occurrences[n_occurrences-1]);
        }
    }

    printf("Full index:\n");
    if (index_print(idx))
    {
        fprintf(stderr, "Error when printing index.\n");
        return 1;
    }

    char new_keyword[17];
    printf("Which keyword do you wish to insert in the index?\n");
    scanf(" %16[^\n]", new_keyword);

    if (index_put(idx, new_keyword))
    {
        fprintf(stderr, "Error when inserting keyword %s.\n", new_keyword);
        return 1;
    }

    printf("Full index updated:\n");
    if (index_print(idx))
    {
        fprintf(stderr, "Error when printing new index.\n");
        return 1;
    }
    return 0;
}

int main(int argc, char** argv)
{
    char* argv_param[] = {"", "keys_file.txt", "text_file.txt"};
    return program2(3, argv_param);
}
