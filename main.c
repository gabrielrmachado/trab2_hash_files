#include <stdio.h>
#include <stdlib.h>
#include "index.h"

void clear_screen()
{
#ifdef _WIN32
    system("cls");
#elif defined(unix) || defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
    system("clear");
#endif
}

void pause_screen()
{
#ifdef _WIN32
    system("pause");
#elif defined(unix) || defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
    fprintf(stdout, "\nThe screen will automatically return to main menu in 5 seconds...\n");
	sleep(5);
#endif
}

int menu()
{
    int op = 0;
    printf("What do you want to do?\n\n");
    printf("1- Create an index;\n");
    printf("2- Update or insert a keyword on the index;\n");
    printf("3- Get the occurrences of a keyword from the index;\n");
    printf("4- Print index;\n");
    printf("5- Exit program.\n");
    printf("\nYour option: "); scanf("%d", &op);
    return op;
}

int program2(int argc, char** argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Error: insufficient number of parameters:\nSintax: %s " "key_file_name txt_file_name\n", argv[0]);
        return 1;
    }

    Index* idx = NULL;
    while (1)
    {
        int option = menu();
        switch (option)
        {
            case 1:
                clear_screen();
                if (index_createfrom(argv[1], argv[2], &idx))
                {
                    fprintf(stderr, "\nError when creating index.\n");
                    return 1;
                }
                else printf("Index created successfully!\n");
                pause_screen();
                break;

            case 2:
                clear_screen();
                char new_keyword[17];

                printf("Type the keyword you wish to insert/update in the index: ");
                scanf(" %16[^\n]", new_keyword);
                if (index_put(idx, new_keyword))
                {
                    fprintf(stderr, "Error when inserting/updating keyword '%s'.\n", new_keyword);
                }
                else printf("Keyword '%s' inserted/updated successfully!\n", new_keyword);
                pause_screen();
                break;

            case 3:
                clear_screen();
                char keyword[17];
                printf("Type the keyword you wish to look for: ");
                scanf(" %16[^\n]", keyword);

                int* occurrences; int n_occurrences;
                if (index_get(idx, keyword, &occurrences, &n_occurrences))
                    fprintf(stderr, "Error: the keyword '%s' does not belong to the index.\n", keyword);
                else
                {
                    if (n_occurrences <= 0)
                        printf("There is no occurrences of the keyword '%s'\n", keyword);
                    else
                    {
                        printf("%d occurrences of '%s'\n", n_occurrences, keyword);
                        for (int i = 0; i < n_occurrences-1; i++)
                            printf("%d, ", occurrences[i]);
                        printf("%d\n", occurrences[n_occurrences-1]);
                    }
                }
                pause_screen();
                break;

            case 4:
                clear_screen();
                printf("Full index:\n\n");
                if (index_print(idx))
                {
                    fprintf(stderr, "Error when printing new index.\n");
                    return 1;
                }
                pause_screen();
                break;

            default:
                exit(1);
        }
    }
}

int program1(int argc, char** argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Error: insufficient number of parameters:\nSintax: %s " "key_file_name txt_file_name\n", argv[0]);
        return 1;
    }

    Index* idx = NULL;
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

    printf("Type the keyword you wish to insert/update in the index: ");
    scanf(" %16[^\n]", new_keyword);
    if (index_put(idx, new_keyword))
    {
        fprintf(stderr, "Error when inserting/updating keyword '%s'.\n", new_keyword);
        return 1;
    }
    else printf("Keyword '%s' inserted/updated successfully!\n", new_keyword);

    printf("Fulled-updated index:\n");
    if (index_print(idx))
    {
        fprintf(stderr, "Error when printing new index.\n");
        return 1;
    }
    return 0;
}

int main(int argc, char** argv)
{
    char* argv_param[] = {"Hash_Index", "keys_file2.txt", "text_file2.txt"};
    return program2(3, argv_param);
//    return program2(argc, argv);
}
