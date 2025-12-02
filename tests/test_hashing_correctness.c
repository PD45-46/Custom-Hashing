#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h> 

#include "../src/ph.h"

void test_basic_correctness() { 

    printf("Running correctness test... \n"); 

    char *keys[] = {"apple", "banana", "carrot", "date", "fig", "grape", "honeydew"};
    size_t n = sizeof(keys)/sizeof(keys[0]); 
    size_t max_str_len = 10;
    
    // Test Regular PH
    ph_table *t_regular = ph_build(keys, n, max_str_len, 0, NULL); 
    
    // All inserted keys should be found
    for(size_t i = 0; i < n; i++) { 
        assert(ph_lookup(t_regular, keys[i]) == 0);
    }
    
    // Non-existent keys should return -1
    assert(ph_lookup(t_regular, "notfound") == -1);
    assert(ph_lookup(t_regular, "xyz") == -1);
    
    ph_free(t_regular);
    
    // Test MPH
    ph_table *t_mph = ph_build(keys, n, max_str_len, 1, NULL);
    
    for(size_t i = 0; i < n; i++) { 
        assert(ph_lookup(t_mph, keys[i]) == 0);
    }
    
    assert(ph_lookup(t_mph, "notfound") == -1);
    
    ph_free(t_mph);
    printf("Basic Correctness Passed!\n\n"); 
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
 *        on the second level it is returning value of 1 (which means there are none -- what we expect).
 *        But  we are trying to measure universal hashing claims of the first level where we fetch from 
 *        a universal set H -> No collisions >= 50% or; Pr(exists collision) <= (N 2)/M < 1/2.   
 */



void test_collision_free() { 
    printf("Running collision-free test... \n"); 

    char *keys[] = {"key1", "key2", "key3", "key4", "key5", 
                    "key6", "key7", "key8", "key9", "key10"};
    size_t n = sizeof(keys)/sizeof(keys[0]); 
    size_t max_str_len = 10; 

    for(int hash_type = 0; hash_type <= 1; hash_type++) {
        ph_table *t = ph_build(keys, n, max_str_len, hash_type, NULL);
        
        // Verify: each key appears exactly once in structure
        int found_count = 0;
        for(size_t i = 0; i < n; i++) {
            if(ph_lookup(t, keys[i]) == 0) {
                found_count++;
            }
        }
        
        assert(found_count == (int)n);
        
        ph_free(t);
    }

    printf("Collision Test Passed!\n\n"); 
}

void stress_test() { 
    printf("Running stress test... \n"); 

    int n = 1000; 
    int max_str_len = 20; 

    char **keys = malloc(n * sizeof(char *));   
    for(int i = 0; i < n; i++) { 
        keys[i] = malloc(max_str_len); 
        snprintf(keys[i], max_str_len, "key_%d", i); 
    }

    for(int hash_type = 0; hash_type <= 1; hash_type++) {
        ph_table *t = ph_build(keys, n, max_str_len, hash_type, NULL);
        
        // Verify all keys can be found
        int errors = 0;
        for(int i = 0; i < n; i++) {
            if(ph_lookup(t, keys[i]) != 0) {
                errors++;
            }
        }
        
        assert(errors == 0);
        
        ph_free(t);
    }

    for(int i = 0; i < n; i++) {
        free(keys[i]);
    }
    free(keys);

    printf("Stress Test Passed!\n\n"); 
}


void test_edge_cases() { 
    printf("Running edge case tests... \n"); 

    // single key 
    char *single[] = {"only"}; 
    ph_table *t1 = ph_build(single, 1, 10, 0, NULL);
    assert(ph_lookup(t1, "only") == 0);
    assert(ph_lookup(t1, "nope") == -1);
    ph_free(t1);

    // two keys 
    char *two[] = {"first", "second"};
    ph_table *t2 = ph_build(two, 2, 10, 0, NULL);
    assert(ph_lookup(t2, "first") == 0);
    assert(ph_lookup(t2, "second") == 0);
    assert(ph_lookup(t2, "third") == -1);
    ph_free(t2);

    // keys w/ common prefix 
    char *prefixes[] = {"test", "testing", "tester", "test123"};
    ph_table *t3 = ph_build(prefixes, 4, 10, 1, NULL);
    for(int i = 0; i < 4; i++) {
        assert(ph_lookup(t3, prefixes[i]) == 0);
    }
    ph_free(t3);

    printf("Edge Cases Passed!\n\n"); 
}

int main()  { 
    srand(time(NULL));
    
    printf("=================================\n");
    printf("Perfect Hashing - Testing \n");
    printf("=================================\n\n");
    
    test_basic_correctness();
    test_collision_free();
    stress_test();
    test_edge_cases();
    
    printf("=================================\n");
    printf("All Tests Passed!\n");
    printf("=================================\n");

    return 0; 
}