#include "cache_perf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>

// Wrapper for perf_event_open syscall
static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                            int cpu, int group_fd, unsigned long flags) {
    return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

// Setup a single perf counter
static int setup_counter(uint32_t type, uint64_t config) {
    struct perf_event_attr pe;
    memset(&pe, 0, sizeof(struct perf_event_attr));
    
    pe.type = type;
    pe.size = sizeof(struct perf_event_attr);
    pe.config = config;
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;
    pe.exclude_idle = 1;
    
    int fd = perf_event_open(&pe, 0, -1, -1, 0);
    if (fd == -1) {
        perror("perf_event_open");
        return -1;
    }
    
    return fd;
}

cache_metrics_t measure_cache_performance(ph_table *ht, char **keys, int n) {
    cache_metrics_t metrics = {0};
    
    // Setup counters
    int fd_cache_ref = setup_counter(PERF_TYPE_HARDWARE, 
                                      PERF_COUNT_HW_CACHE_REFERENCES);
    int fd_cache_miss = setup_counter(PERF_TYPE_HARDWARE, 
                                       PERF_COUNT_HW_CACHE_MISSES);
    
    // L1 data cache - this is more complex, using cache event encoding
    // PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8) | 
    // (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16)
    int fd_l1_loads = setup_counter(PERF_TYPE_HW_CACHE, 
                                     0x0 | (0x0 << 8) | (0x0 << 16));
    int fd_l1_misses = setup_counter(PERF_TYPE_HW_CACHE, 
                                      0x0 | (0x0 << 8) | (0x1 << 16));
    
    if (fd_cache_ref == -1 || fd_cache_miss == -1) {
        fprintf(stderr, "Warning: Could not setup cache performance counters\n");
        fprintf(stderr, "Try running with: sudo or setting /proc/sys/kernel/perf_event_paranoid to -1\n");
        return metrics;
    }
    
    // Reset and enable counters
    ioctl(fd_cache_ref, PERF_EVENT_IOC_RESET, 0);
    ioctl(fd_cache_miss, PERF_EVENT_IOC_RESET, 0);
    if (fd_l1_loads != -1) ioctl(fd_l1_loads, PERF_EVENT_IOC_RESET, 0);
    if (fd_l1_misses != -1) ioctl(fd_l1_misses, PERF_EVENT_IOC_RESET, 0);
    
    ioctl(fd_cache_ref, PERF_EVENT_IOC_ENABLE, 0);
    ioctl(fd_cache_miss, PERF_EVENT_IOC_ENABLE, 0);
    if (fd_l1_loads != -1) ioctl(fd_l1_loads, PERF_EVENT_IOC_ENABLE, 0);
    if (fd_l1_misses != -1) ioctl(fd_l1_misses, PERF_EVENT_IOC_ENABLE, 0);
    
    // Perform lookups
    for (int i = 0; i < n; i++) {
        ph_lookup(ht, keys[i]);
    }
    
    // Disable counters
    ioctl(fd_cache_ref, PERF_EVENT_IOC_DISABLE, 0);
    ioctl(fd_cache_miss, PERF_EVENT_IOC_DISABLE, 0);
    if (fd_l1_loads != -1) ioctl(fd_l1_loads, PERF_EVENT_IOC_DISABLE, 0);
    if (fd_l1_misses != -1) ioctl(fd_l1_misses, PERF_EVENT_IOC_DISABLE, 0);
    
    // Read results
    read(fd_cache_ref, &metrics.cache_references, sizeof(long long));
    read(fd_cache_miss, &metrics.cache_misses, sizeof(long long));
    
    if (fd_l1_loads != -1 && fd_l1_misses != -1) {
        read(fd_l1_loads, &metrics.l1_dcache_loads, sizeof(long long));
        read(fd_l1_misses, &metrics.l1_dcache_misses, sizeof(long long));
    }
    
    // Calculate miss rates
    if (metrics.cache_references > 0) {
        metrics.cache_miss_rate = (double)metrics.cache_misses / 
                                   metrics.cache_references * 100.0;
    }
    
    if (metrics.l1_dcache_loads > 0) {
        metrics.l1_miss_rate = (double)metrics.l1_dcache_misses / 
                                metrics.l1_dcache_loads * 100.0;
    }
    
    // Close file descriptors
    close(fd_cache_ref);
    close(fd_cache_miss);
    if (fd_l1_loads != -1) close(fd_l1_loads);
    if (fd_l1_misses != -1) close(fd_l1_misses);
    
    return metrics;
}

void print_cache_metrics(cache_metrics_t *metrics, int n) {
    printf("\n--- CACHE PERFORMANCE ---\n");
    printf("  Cache references:   %lld\n", metrics->cache_references);
    printf("  Cache misses:       %lld\n", metrics->cache_misses);
    printf("  Cache miss rate:    %.2f%%\n", metrics->cache_miss_rate);
    
    if (metrics->l1_dcache_loads > 0) {
        printf("  L1 D-cache loads:   %lld\n", metrics->l1_dcache_loads);
        printf("  L1 D-cache misses:  %lld\n", metrics->l1_dcache_misses);
        printf("  L1 miss rate:       %.2f%%\n", metrics->l1_miss_rate);
    }
    
    printf("  Avg cache refs per lookup: %.2f\n", 
           (double)metrics->cache_references / n);
    printf("  Avg cache misses per lookup: %.2f\n", 
           (double)metrics->cache_misses / n);
}