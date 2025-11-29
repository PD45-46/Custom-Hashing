#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../src/ph.h"

/**
 * @brief Generate s a bunch of random keys 
 */

char **generate_keys(unsigned int n, unsigned int len) { 
    char **keys = malloc(n * sizeof(char *)); 
    for(size_t i = 0; i < n; i++) { 
        keys[i] = malloc(len); 
        for(size_t j = 0; j < len - 1; j++) { 
            keys[i][j] = 'a' + rand()%26; 
        }
        keys[i][len - 1] = '\0';  
    }

    // for(size_t i = 0; i < n; i++) { 
    //     printf("Generated key num %ld/%d: %s\n", i, n, keys[i]); 
    // }

    return  keys; 
}

void free_keys(char **keys, int n) { 
    for(int i = 0; i < n; i++) free(keys[i]); 
    free(keys); 
}

/** 
 * @brief In some cases, the generate_keys() function will produce 
 *        multiple identical keys in the same string array and so we need 
 *        to turn the array into a set by removing duplicate instances. 
 *        Note that the most common cases in which this occurs; 
 *          - When the number of generated keys is larger than all the possible 
 *            combinations using key length.
 *          - When two or more identical keys are made purely due to randomness 
 *            in generate_keys(). 
 * 
 */
static char **key_set_cleaner(char **keys, int *n) {
    int old_n = *n;
    int count = 0;

    for (int i = 0; i < old_n; i++) {
        int dup = 0;
        for (int j = 0; j < count; j++) {
            if (strcmp(keys[i], keys[j]) == 0) {
                dup = 1;
                free(keys[i]); 
                break;
            }
        }
        if (!dup) {
            keys[count++] = keys[i];
        }
    }

    *n = count;
    keys = realloc(keys, count * sizeof(char*));

    
    if (count != old_n) {
        printf("[key_set_cleaner] Removed %d duplicate keys (from %d → %d)\n",
               old_n - count, old_n, count);
    }

    return keys;
}

void benchmark_ph(int n, int key_len) { 
    char **keys = generate_keys(n, key_len); 

    keys = key_set_cleaner(keys, &n); 

    clock_t start = clock(); 
    ph_table *ht = ph_build(keys, n, key_len); 
    clock_t end = clock(); 
    double build_time = (double)(end - start)/CLOCKS_PER_SEC; 
    printf("PH Build Time for %d keys and %d key len: %f sec\n", n, key_len, build_time); 
    
    // lookup benchmark 
    start = clock(); 
    for(int i = 0; i < n; i++) { 
        int found = ph_lookup(ht, keys[i]); 
        if(found == -1) printf("Error: Key not found\n"); 
    }
    end = clock(); 
    double lookup_time = (double)(end - start)/CLOCKS_PER_SEC; 
    printf("PH Lookup Time for %d keys: %f sec\n", n, lookup_time); 

    ph_free(ht); 
    free_keys(keys, n); 
}

int main(int argc, char *argv[]) { 
    if(argc != 3) { 
        printf("Invalid num of args"); 
        return 0; 
    }
    srand(time(NULL)); 
    int n = atoi(argv[1]); 
    int key_len = atoi(argv[2]); 

    benchmark_ph(n, key_len); 
    return 0; 
}