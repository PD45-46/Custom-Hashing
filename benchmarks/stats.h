#ifndef STATS_H
#define STATS_H

#include <stddef.h>

typedef struct { 
    double min; 
    double max; 
    double median; 
    double mean; 
    double p95; 
    double p99; 
    double std_dev; 
} stats_t; 

int compare_doubles(const void *a, const void *b); 
double calc_min(double *values, size_t n); 
double calc_max(double *values, size_t n); 
double calc_mean(double *values, size_t n); 
double calc_median(double *values, size_t n); 
double calc_percentile(double *values, size_t n, int percentile); 
double calc_stddev(double *values, size_t n, double mean); 
stats_t calc_stats(double *values, size_t n); 

#endif