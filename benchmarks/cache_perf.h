#ifndef CACHE_PERF_H
#define CACHE_PERF_H

#include <stdint.h>
#include "../src/ph.h"

typedef struct {
    long long cache_references;
    long long cache_misses;
    long long l1_dcache_loads;
    long long l1_dcache_misses;
    double cache_miss_rate;
    double l1_miss_rate;
} cache_metrics_t;

// Initialize cache performance counters
int init_cache_counters(int *fds, int num_counters);

// Measure cache performance during lookups
cache_metrics_t measure_cache_performance(ph_table *ht, char **keys, int n);

// Print cache metrics
void print_cache_metrics(cache_metrics_t *metrics, int n);

#endif