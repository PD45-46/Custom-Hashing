#ifndef PH_H
#define PH_H

#include <stddef.h> 

typedef struct { 
    
    unsigned int prime; 
    unsigned int table_size; 
    unsigned int rand_additive; 
    unsigned int max_str_len; 
    unsigned int* coeff_array; 

} Universal_Hash_Params; 

typedef struct { 
    char **keys; 
    size_t key_count; 
    size_t table_size; 
    Universal_Hash_Params params; 
} ph_bucket_t; 

typedef struct { 
    size_t n; // num of keys in total
    size_t m; // num of total buckets
    ph_bucket_t *buckets; // array of buckets 
    Universal_Hash_Params level1_params;
} ph_table; 

/**
 * @brief 
 * 
 * */
ph_table *ph_build(char **keys, size_t n, size_t max_str_len);

/** 
 * @brief Look up a key in the hash table t in the index... 
 */
int ph_lookup(ph_table *t, const char *key);

/* Frees all mem */
void ph_free(ph_table *t); 

#endif
