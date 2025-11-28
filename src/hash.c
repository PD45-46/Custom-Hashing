#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ph.h"
#include "hash.h"

#define PRIME 2147483647u





/** 
 * 
 * @brief A randomised algo for constructing hash functions 
 *        such that the prob of collision is ~ 1/M (theoretically). 
 * 
 * @param key - The String to hash 
 * @param rand_coeff - The array of random coefficients (a[i])
 * @param rand_additive_const - The random additive constant (b)
 * @param prime - The chosen (large) prime value to perform mod with 
 * @param table_size - Size of our hash table
 * 
 * @return integer hash value between [0, table_size] 
 * 
 */
unsigned int universal_hash(char* key, Universal_Hash_Params* params) { 
    
    unsigned long long hash = params->rand_additive; 

    for(int i = 0;  key[i] != '\0'; i++) { 
        unsigned int temp = (unsigned char) key[i]; 
        hash += (unsigned long long)temp * params->coeff_array[i]; 
        // optional - reduce mod p to avoid overflow
    }
    return (unsigned int)((hash % params->prime) % params->table_size);
}

/** 
 * 
 * @brief Initialises the params for our universal hash function and 
 *        sets key size limits and table size limits. Note that it 
 *        also creates the random coefficient array that is used to 
 *        ensure low prob of collisions (primarily <= 1/table_size). 
 * 
 * @param params Location to store all params 
 * @param table_size size of to be hash table 
 * @param max_str_len max length of future input keys 
 * 
 */
void init_universal_hash(Universal_Hash_Params* params, unsigned int table_size, unsigned int max_str_len) { 
    
    params->prime = PRIME; 
    params->table_size = table_size; 
    params->max_str_len = max_str_len; 

    params->coeff_array = malloc(sizeof(unsigned int) * max_str_len); 

    params->rand_additive = (unsigned int)(rand() % params->prime); 

    for(int i = 0; i < max_str_len; i++) { 
        unsigned int r = (unsigned int) rand(); 
        params->coeff_array[i] = (r % (params->prime - 1)) + 1; 
    }
}

void free_universal_hash(Universal_Hash_Params* params) { 
    free(params->coeff_array); 
    params->coeff_array = NULL; 
}

void build_first_level_bucketing(ph_table *t, char **keys, size_t n, size_t max_str_len) { 

    t->n = n; 
    // change later for 2nd method 
    t->m = n; 
    t->buckets = calloc(t->n, sizeof(ph_bucket_t)); 
    
    init_universal_hash(&t->level1_params, t->m, max_str_len); 

    // change later for 2nd method
    for(size_t i = 0; i < n; i++) { 
        unsigned int h = universal_hash(keys[i], &t->level1_params) % t->m; 
        ph_bucket_t *b = &t->buckets[h]; 
        
        // expand bucket key list  
        b->keys = realloc(b->keys, sizeof(char *) * (b->key_count + 1)); 
        b->keys[b->key_count++] = keys[i]; 
    }
}


void build_second_level_bucketing(ph_bucket_t *b, size_t max_str_len) { 

    size_t k = b->key_count; 

    if(k <= 1) {  // trivial case
        b->table_size = k; 
        return; 
    }

    size_t m2 = k * k; // perfect hashing method 1 guarantee O(N^2)
    char **table = NULL; 

    while(1) { 
        init_universal_hash(&b->params, m2, max_str_len); 
        table = calloc(m2, sizeof(char *)); 
        int collision = 0; 



        for(size_t i = 0; i < k; i++) { 
            unsigned int h = universal_hash(b->keys[i], &b->params);
            
            if(table[h] != NULL) { 
                collision = 1; 
                break; 
            }

            table[h] = b->keys[i]; 
        }

        if(!collision) { 
            free(b->keys); 
            b->keys = table; 
            b->table_size = m2; 
            return; 
        }

        free(table); 
    }





}


ph_table *ph_build(char **keys, size_t n, size_t max_str_len) { 
    ph_table *t = calloc(1, sizeof(ph_table)); 
    build_first_level_bucketing(t, keys, n, max_str_len); 
    for(size_t i = 0; i < t->m; i++) { 
        build_second_level_bucketing(&t->buckets[i], max_str_len); 
    }
    return t; 
}

int ph_lookup(ph_table *t, const char *key) { 
    
    unsigned int h1 = universal_hash(key, &t->level1_params) % t->m; 
    ph_bucket_t *b = &t->buckets[h1]; 

    if(b->key_count == 0) return -1; 
    if(b->key_count == 1) { 
         return strcmp(b->keys[0], key) == 0 ? 0 : -1; 
    }

    unsigned int h2 = universal_hash(key, &b->params) % b->table_size; 
    return (b->keys[h2] && strcmp(b->keys[h2], key) == 0) ? 0 : -1; 
}

void ph_free(ph_table *t) { 
    if(!t) return; 

    free_universal_hash(&t->level1_params); 
    for(size_t i = 0; i < t->m; i++) { 
        ph_bucket_t *b =  &t->buckets[i]; 
        free_universal_hash(&b->params); 
        free(b->keys); 
    }
    free(t->buckets); 
    free(t); 
}


