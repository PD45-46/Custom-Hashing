#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../src/ph.h"

void test_ph1_correctness() { 

    char *keys[] = {"apple" , "banana", "carrot", "date", "fig", "grape", "honeydew"};
    size_t n = sizeof(keys)/sizeof(keys[0]); 
    size_t max_str_len = 10;
    ph_table *t = ph_build(keys, n, max_str_len); 

    int errors = 0; 
    for(size_t i = 0; i < n; i++) { 
        if(ph_lookup(t, keys[i]) != 0) {  
            printf("Failed to find key: %s\n" , keys[i]); 
            errors++; 
        }
    }

    if(ph_lookup(t, "not_in_table" ) != -1) { 
        printf("Error: found key that shouldn't exist\n");
        errors++;
    }

    if (errors == 0) {
        printf("Correctness test passed!\n");
    }

    ph_free(t);
}

// Prob sim

/** 
 * @brief Theory states that "If H is universal and M = N^2, then Pr(no collisions in S) >= 1/2 
 *        when picking a random hash function." The output should converge to >= 50%
 * 
 * @param trials The num of times we want to perform universal hashing 
 * 
 * @param n Number of keys to create and store 
 * 
 * TODO - Change so that we dont always ret value 1. Because we measure the mount of collisions 
 *        on the second level it is returning val ue of 1 (which means there are none -- what we expect).
 *        But  we are trying to measure universal hashing claims of the first level where we fetch from 
 *        a universal set H -> No collisions >= 50% or; Pr(exists collision) <= (N 2)/M < 1/2.   
 */

void test_prob(int trials, int n) { 
    int success = 0; 
    int max_str_len = 3; //  26^2 = 676 possible keys 

    // generate n random keys 
    char **keys = malloc(n * sizeof(char *)); 
    for(int i = 0; i < n; i++) { 
        keys[i] = malloc(max_str_len); 
        for(int j = 0; j < max_str_len - 1; j++) { 
            keys[i][j] = 'a' + (rand()%26); 
        }
        keys[i][max_str_len-1]  = '\0'; 
    }
    keys[0] = strdup("aaa"); 
    keys[1] = strdup("aaa"); 

    for(int t = 0; t < trials; t++) { 
        ph_table *ht = ph_build(keys, n, max_str_len); 
        int collisions = 0; 

        // check each second-level table for collisions 
        for(size_t i = 0; i < ht->m; i++) { 
            ph_bucket_t *b = &ht->buckets[i]; 
            if(b->key_count > 1) { 
                for(size_t j = 0; j < b->table_size; j++) { 
                    if(b->keys[j] == NULL) continue; 
                    for(size_t k = j + 1; k < b->table_size; k++) { 
                        if(b->keys[k] && strcmp(b->keys[j], b-> keys[k]) == 0) collisions++; 
                    }
                }
            }
        }
        if(collisions == 0) success++; 
        ph_free(ht); 
    }
    printf("No-collision prob: %f\n", (double)success/trials); 
    for(int i = 0; i < n; i++) { 
        free(keys[i]); 
    }
    free(keys); 

}

int main(int argc, char *argv[])  { 
    if(argc != 3) { 
        printf("Wrong number of args\n"); 
        return 0; 
    }

    test_ph1_correctness(); 
    test_prob(atoi(argv[1]), atoi(argv[2])); 

    return 0; 
}