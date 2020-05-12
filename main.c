#include <stdio.h>
#include "index.h"

int main(int argc, char** argv)
{
    Index* idx;
    argv[1] = "keys_file.txt";
    argv[2] = "text_file.txt";
    index_createfrom(argv[1], argv[2], &idx);
    index_print(idx);
    index_put(idx, "read"); printf("\n");
    index_print(idx);

    return 0;
}
