#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../src/ph.h"
#include "stats.h"
#include "cache_perf.h"

#define NUM_TRIALS 10
#define WARMUP_RUNS 3

typedef struct {
    double build_time;
    double lookup_time;
    size_t memory_bytes;
    build_metrics_t build_metrics;
    cache_metrics_t cache_metrics; 
} trial_result_t;



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

double get_time_seconds() { 
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts); 
    return ts.tv_sec + ts.tv_nsec / 1e9; 
}

/** 
 * @brief Calculates the amount of memory space used up 
 *        by a hash table. 
 */
size_t calc_mem(ph_table *t, size_t max_str_len) { 
    size_t total = 0; 

    // Table 
    total += sizeof(ph_table); 

    // Level 1 bucket 
    total += t->m * sizeof(ph_bucket_t); 

    // Hash params 
    total += sizeof(Universal_Hash_Params); 
    total += max_str_len * sizeof(unsigned int); 

    // Level 2 bucket 
    for(size_t i = 0; i < t->m; i++) { 
        ph_bucket_t *b = &t->buckets[i]; 
        if(b->key_count == 0) continue;
        total += b->table_size * sizeof(char *);
        if(b->key_count > 1) { 
            total += sizeof(Universal_Hash_Params); 
            total += max_str_len * sizeof(unsigned int); 
        }
    } 
    return total; 
}

trial_result_t single_trial(int n, int key_len, int hash_type) { 
    trial_result_t result = {0}; 

    char **keys = generate_keys(n, key_len); 
    int original_n = n; 
    keys = key_set_cleaner(keys, &n); 

    double start = get_time_seconds(); 
    ph_table *ht = ph_build(keys, n, key_len, hash_type, &result.build_metrics); 
    double end = get_time_seconds(); 
    result.build_time = end - start; 

    result.memory_bytes = calc_mem(ht, key_len); 

    start = get_time_seconds(); 
    for(int i = 0; i < n; i++) { 
        int found = ph_lookup(ht, keys[i]); 
        if(found == -1) { 
            printf("Error: Key '%s'  not found\n", keys[i]); 
        }
    }
    end = get_time_seconds(); 
    result.lookup_time = (end - start) / n; //  per key avg 
    // result.cache_metrics = measure_cache_performance(ht, keys, n); 

    ph_free(ht); 
    free_keys(keys, n);
    
    return result; 
}

/** 
 * @brief Note that we use warmup runs to ensure that caches contain information for 
 *        all runs to ensure consistency between multiple trials. I've seen that 
 *        the first trial always seems to be a little slower than subsequent ones given 
 *        the same params and which is why i added the warmup. 
 */
void benchmark_ph(int n, int key_len, int hash_type) { 
    printf("========================================\n");
    if(hash_type == 0) {
        printf("\033[32mHash Type: Regular Perfect Hashing (O(n^2))\033[0m\n");
    } else {
        printf("\033[36mHash Type: Minimal Perfect Hashing (O(n))\033[0m\n");
    }
    printf("Dataset: %d keys, %d chars per key\n", n, key_len);
    printf("========================================\n");

    // Warmup runs 
    printf("Running %d warmup trial runs... \n", WARMUP_RUNS); 
    for(int i = 0; i < WARMUP_RUNS; i++) { 
        trial_result_t warmup = single_trial(n, key_len, hash_type); 
        (void)warmup; // supress the unused warning
    }

    printf("Running %d benchmark trial runs... \n", NUM_TRIALS); 
    double *build_times = malloc(NUM_TRIALS * sizeof(double)); 
    double *lookup_times = malloc(NUM_TRIALS * sizeof(double)); 
    size_t *memory_sizes = malloc(NUM_TRIALS * sizeof(size_t)); 
    int *total_attempts = malloc(NUM_TRIALS * sizeof(int));
    int *max_attempts = malloc(NUM_TRIALS * sizeof(int));

    // long long *cache_refs = malloc(NUM_TRIALS * sizeof(long long)); 
    // long long *cache_misses = malloc(NUM_TRIALS * sizeof(long long)); 
    // double *cache_miss_rates = malloc(NUM_TRIALS * sizeof(double)); 
    
    for(int trial = 0; trial < NUM_TRIALS; trial++) { 
        trial_result_t result = single_trial(n, key_len, hash_type); 
        build_times[trial] = result.build_time; 
        lookup_times[trial] = result.lookup_time; 
        memory_sizes[trial] = result.memory_bytes; 
        total_attempts[trial] = result.build_metrics.total_attempts; 
        max_attempts[trial] = result.build_metrics.max_attemps_bucket; 

        // cache_refs[trial] = result.cache_metrics.cache_references;
        // cache_misses[trial] = result.cache_metrics.cache_misses;
        // cache_miss_rates[trial] = result.cache_metrics.cache_miss_rate;

        printf("Trial %d: build=%.6fs, lookup=%.9fs, mem=%zuKB, cache_miss=%.2f%%\n", 
            trial + 1, result.build_time, result.lookup_time, result.memory_bytes / 1024, result.cache_metrics.cache_miss_rate); 
    }

    stats_t build_stats = calc_stats(build_times, NUM_TRIALS); 
    stats_t lookup_stats = calc_stats(lookup_times, NUM_TRIALS); 

    double mem_vals[NUM_TRIALS]; 
    double attempts_vals[NUM_TRIALS];
    // double cache_ref_vals[NUM_TRIALS];
    // double cache_miss_vals[NUM_TRIALS];

    for(int i = 0; i < NUM_TRIALS; i++) { 
        mem_vals[i] = (double)memory_sizes[i];
        attempts_vals[i] = (double)total_attempts[i]; 
        // cache_ref_vals[i] = (double)cache_refs[i];
        // cache_miss_vals[i] = (double)cache_misses[i];
    }
    stats_t mem_stats = calc_stats(mem_vals, NUM_TRIALS);   
    stats_t attempts_stats = calc_stats(attempts_vals, NUM_TRIALS);
    // stats_t cache_miss_rate_stats = calc_stats(cache_miss_rates, NUM_TRIALS);
    // stats_t cache_ref_stats = calc_stats(cache_ref_vals, NUM_TRIALS);
    // stats_t cache_miss_stats = calc_stats(cache_miss_vals, NUM_TRIALS);
    
    printf("\n--- BUILD TIME (seconds) ---\n");
    printf("  Min:    %.6f\n", build_stats.min);
    printf("  Median: %.6f\n", build_stats.median);
    printf("  Mean:   %.6f\n", build_stats.mean);
    printf("  P95:    %.6f\n", build_stats.p95);
    printf("  P99:    %.6f\n", build_stats.p99);
    printf("  Max:    %.6f\n", build_stats.max);
    printf("  StdDev: %.6f\n", build_stats.std_dev);
    
    printf("\n--- LOOKUP TIME (seconds per key) ---\n");
    printf("  Min:    %.9f\n", lookup_stats.min);
    printf("  Median: %.9f\n", lookup_stats.median);
    printf("  Mean:   %.9f\n", lookup_stats.mean);
    printf("  P95:    %.9f\n", lookup_stats.p95);
    printf("  P99:    %.9f\n", lookup_stats.p99);
    printf("  Max:    %.9f\n", lookup_stats.max);
    
    printf("\n--- MEMORY USAGE ---\n");
    printf("  Median: %zu bytes (%.2f KB, %.2f MB)\n",
           (size_t)mem_stats.median,
           mem_stats.median / 1024.0,
           mem_stats.median / (1024.0 * 1024.0));
    printf("  Per key: %.2f bytes\n", mem_stats.median / n);
    
    printf("\n--- BUILD METRICS ---\n");
    printf("  Avg total attempts: %.1f\n", attempts_stats.mean);
    printf("  Max attempts (worst bucket): %d\n", max_attempts[0]);

    // printf("\n--- CACHE PERFORMANCE ---\n");
    // printf("  Median cache references: %.0f\n", cache_ref_stats.median);
    // printf("  Median cache misses: %.0f\n", cache_miss_stats.median);
    // printf("  Mean cache miss rate: %.2f%%\n", cache_miss_rate_stats.mean);
    // printf("  Median cache miss rate: %.2f%%\n", cache_miss_rate_stats.median);
    // printf("  P95 cache miss rate: %.2f%%\n", cache_miss_rate_stats.p95);
    // printf("  Avg cache refs per lookup: %.2f\n", cache_ref_stats.mean / n);
    // printf("  Avg cache misses per lookup: %.2f\n", cache_miss_stats.mean / n);
    
    // Cleanup
    free(build_times);
    free(lookup_times);
    free(memory_sizes);
    free(total_attempts);
    free(max_attempts);
    // free(cache_refs);
    // free(cache_misses);
    // free(cache_miss_rates);
}



int main(int argc, char *argv[]) { 
    if(argc != 3) { 
        printf("Invalid num of args"); 
        return 0; 
    }
    srand(time(NULL)); 
    int n = atoi(argv[1]); 
    int key_len = atoi(argv[2]); 

    benchmark_ph(n, key_len, 0);
    benchmark_ph(n, key_len, 1); 
    return 0; 
}