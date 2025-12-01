#include "stats.h"
#include <stdlib.h>
#include <math.h>

int compare_doubles(const void *a, const void *b) { 
    double diff = (*(double *)a - *(double *)b); 
    return (diff > 0) - (diff < 0); 
}

double calc_min(double *values, size_t n) { 
    double min = values[0]; 
    for(size_t i = 1; i < n; i++) { 
        if(min > values[i]) min = values[i]; 
    }
    return min; 
}

double calc_max(double *values, size_t n) { 
    double max = values[0]; 
    for(size_t i = 1; i < n; i++) { 
        if(max < values[i]) max = values[i]; 
    }
    return max; 
}

double calc_mean(double *values, size_t n) { 
    double mean = 0; 
    for(size_t i = 0; i < n; i++) { 
        mean += values[i]; 
    }
    return mean/n; 
}

double calc_median(double *values, size_t n) { 
    
    double *sorted = malloc(n * sizeof(double)); 
    for(size_t i = 0; i < n; i++) {
        sorted[i] = values[i];
    }

    qsort(sorted, n, sizeof(double), compare_doubles); 

    double median; 
    if(n % 2 == 0) { 
        median = (sorted[n/2 - 1] + sorted[n/2]) / 2.0; 
    } else { 
        median = sorted[n/2]; 
    }

    free(sorted); 
    return median; 
}

double calc_percentile(double *values, size_t n, int percentile) { 
    double *sorted = malloc(n * sizeof(double));
    for(size_t i = 0; i < n; i++) {
        sorted[i] = values[i];
    }
    
    qsort(sorted, n, sizeof(double), compare_doubles);
    
    size_t index = (size_t)((percentile / 100.0) * (n - 1));
    double result = sorted[index];
    
    free(sorted);
    return result;
}

double calc_stddev(double *values, size_t n, double mean) { 
    double sum_squared_diff = 0.0; 
    for(size_t i = 0; i < n; i++) { 
        double diff = values[i] - mean; 
        sum_squared_diff += diff * diff; 
    }
    return sqrt(sum_squared_diff/n); 
}

stats_t calc_stats(double *values, size_t n) { 
    stats_t stats; 
    stats.min = calc_min(values, n); 
    stats.max = calc_max(values, n); 
    stats.mean = calc_mean(values, n);
    stats.median = calc_median(values, n); 
    stats.p95 = calc_percentile(values, n, 95); 
    stats.p99 = calc_percentile(values, n, 99);
    stats.std_dev = calc_stddev(values, n, stats.mean); 
    return stats;  
}






