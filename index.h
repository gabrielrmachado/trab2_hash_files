//
// Created by gabri on 24/04/2020.
//

#ifndef HASH_INDEX_INDEX_H

typedef struct index Index;

int index_createfrom(const char* key_file, const char* text_file, Index** idx);
int index_get(const Index* idx, const char* key, int** occurrences, int* num_ocurrences);
int index_put(const Index* idx, const char* key);
int index_print(const Index* idx);

#define HASH_INDEX_INDEX_H

#endif //HASH_INDEX_INDEX_H
