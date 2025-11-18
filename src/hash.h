#ifndef HASH_H
#define HASh_H

#include <stddef.h>

typedef struct { 
    
    unsigned int prime; 
    unsigned int table_size; 
    unsigned int rand_additive; 
    unsigned int max_str_len; 
    unsigned int* coeff_array; 

} Universal_Hash_Params; 

unsigned int first_level_hashing(const char *s); 
unsigned int second_level_hashing(const char *s, unsigned int seed);

#endif
