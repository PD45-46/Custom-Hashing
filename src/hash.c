#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ph.h"
#include "hash.h"

#define PRIME 2147483647u

/* Build first level buckets */
static void build_buckets(ph_table *t, char **keys, size_t n) { 
    // one bucket per key (on avg)
    t->m = n; 
    t->buckets = calloc(n, sizeof(ph_bucket_t)); // safer than malloc

    for(size_t i = 0; i < n; i++) { 
        unsigned int h = fnv1a(keys[i]) % n; 
        ph_bucket_t *b = &t->buckets[h]; 

        // grow the bucket's key list 
        b->keys = realloc(b->keys, (b->size + 1) * sizeof(char *)); 
        b->keys[b->size++] = keys[i]; 
    }
}

/* Allocate collision-free second level hash table */
static void build_second_level(ph_bucket_t *b) { 
    if(b->size <= 1) { 
        return; 
    }

    size_t s = b->size * b->size; 

    char **table = NULL; 

    while(1) { 
        unsigned int seed = rand(); 
        table = calloc(s, sizeof(char *)); 
        int collision = 0; 

        for(size_t i = 0; i < b->size; i++) { 
            unsigned int h = seeded_hash(b->keys[i], seed) % s; 
            if(table[h] != NULL) { 
                collision = 1;
                // retry with new seed 
                break; 
            }
            table[h] = b->keys[i]; 
        }

        if(!collision) { 
            b->seed = seed; 
            free(b->keys); 
            b->keys = table; 
            b->size = s; 
            return; 
        }
        free(table); 
    }
}

ph_table *ph_build(char **keys, size_t n) { 
    ph_table *t = calloc(1, sizeof(ph_table)); 
    t->n = n; 

    build_buckets(t, keys, n); 

    for(size_t i = 0; i < t->m; i++) { 
        build_second_level(&t->buckets[i]); 
    }

    return t; 
}

int ph_lookup(ph_table *t, const char *key, size_t *out_index) { 
    unsigned int h1 = fnv1a(key) % t->n; 
    ph_bucket_t *b = &t->buckets[h1];
    
    if(b->size == 1) { 
        if(strcmp(b->keys[0], key) == 0) { 
            *out_index = h1;  
            return 0; 
        }
        return -1; 
    }

    unsigned int h2 = seeded_hash(key, b->seed) % b->size; 

    if(b->keys[h2] && strcmp(b->keys[h2], key) == 0) {  
        *out_index = h2; 
        return 0; 
    }
    return -1; 
}

void ph_free(ph_table *t) { 
    for(size_t i = 0; i < t->m; i++) { 
        free(t->buckets[i].keys); 
    }
    free(t->buckets); 
    free(t); 
}





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