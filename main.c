#include <stdio.h>
#include "index.h"

int main(int argc, char** argv)
{
    Index* idx;
    argv[1] = "keys_file.txt";
    index_createfrom(argv[1], argv[2], &idx);

    return 0;
}
