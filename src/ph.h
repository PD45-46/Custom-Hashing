#ifndef PH_H
#define PH_H

#include <stddef.h> 
#include "hash.h"

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
ph_table *mph_build(char **keys, size_t n); 

/** 
 * @brief Look up a key in the hash table t in the index... 
 */
int mph_lookup(ph_table *t, const char *key, size_t *out_index); 

/* Frees all mem */
void mph_free(ph_table *t); 

#endif
