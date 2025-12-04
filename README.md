# Perfect Hashing: Performance Analysis of O(n²) vs O(n) Implementations

## Overview

This project implements and benchmarks two perfect hashing algorithms in C: Regular Perfect Hashing (O(n²) space) and Minimal Perfect Hashing (O(n) space). The implementation is based on content from Australian National University's COMP3600 (Algorithms) and Carnegie Mellon University's [Universal and Perfect Hashing Lecture](https://www.cs.cmu.edu/~avrim/451f11/lectures/lect1004.pdf).

## Background

### Hashing Fundamentals

Hashing provides O(1) average-case lookup by mapping keys to array indices using a hash function. A basic hash function takes the form:
```
h(x) = (ax + b) mod p
```

Where `a` and `b` are constants, `x` is the input key, and `p` is a prime number.

### Universal Hashing

Universal hashing reduces collision probability by randomly selecting hash functions from a family. For any pair of keys, the collision probability is ≤ 1/m:
```
h(x) = ((ax + b) mod p) mod m
```

Where `p` is a large prime (p > m), `a` and `b` are random values in [1, p-1], and `m` is the hash table size.

### Perfect Hashing

Perfect hashing eliminates collisions entirely using a two-level scheme:
- **Level 1:** Distribute n keys into n buckets
- **Level 2:** Each bucket uses a secondary hash table sized to guarantee zero collisions

**Regular Perfect Hashing:** Uses O(n²) total space with secondary tables of size k²  
**Minimal Perfect Hashing:** Uses O(n) total space with secondary tables of size k

## Implementation

### Two-Level Hashing Scheme

**Level 1:** Hash n keys into n buckets using universal hashing  
**Level 2:** Each bucket with k keys uses secondary hash table of size m₂
- Regular PH: m₂ = k² (guarantees collision-free in 1-2 attempts)
- MPH: m₂ = k (minimal space, requires multiple retry attempts)

### Universal Hash Function
```
h(x) = ((Σ aᵢ·xᵢ + b) mod p) mod m
```

**Parameters:**
- Prime p = 2,147,483,647 (Mersenne prime M₃₁)
- Random coefficients aᵢ ∈ [1, p-1] per hash function
- Random constant b ∈ [0, p-1]
- Collision probability ≤ 1/m for any key pair

**Design decisions:**
- Fixed coefficient array size at `max_str_len` for variable-length strings
- `unsigned long long` for intermediate computation prevents overflow
- Double modulo ensures uniform distribution

### Construction Algorithm

**Build process:**
1. Distribute n keys into n first-level buckets (O(n) expected)
2. For each bucket with k > 1 keys:
   - Generate random universal hash function
   - Attempt collision-free placement in m₂ slots
   - On collision: regenerate hash parameters and retry
   
**Retry characteristics:**
- Regular PH: 1-2 attempts expected (abundant space)
- MPH: 10-100+ attempts expected (tight space constraint)

### Lookup Operation

**Two-level lookup (O(1) worst-case):**
1. Compute h₁ = hash(key) mod m to find bucket
2. If bucket empty, return not found
3. If bucket has single key, perform direct comparison
4. Compute h₂ = hash(key) mod m₂ for secondary table
5. Compare key at position h₂

**Performance:** Two hash computations + one string comparison per lookup, with no collision resolution required.

### Memory Management

**Space complexity:**
- Regular PH: O(n²) worst-case
- MPH: O(n) 

**Allocation strategy:**
- Level 1: Contiguous allocation for n buckets
- Level 2: Per-bucket allocations for secondary tables
- Hash parameters: Coefficient arrays per table/bucket
- Keys: Pointer storage only (no string duplication)

### Code Organization

**Core functions:**
- `universal_hash()` - Hash computation
- `init_universal_hash()` - Random parameter generation
- `build_first_level_bucketing()` - Initial key distribution
- `build_second_level_bucketing()` - Per-bucket collision-free construction
- `ph_build()` - Main build coordinator with metrics
- `ph_lookup()` - Two-level lookup with edge cases
- `ph_free()` - Memory cleanup

**Tracked metrics:**
- Total hash function attempts
- Maximum attempts for worst-case bucket
- Collision count during construction

## Testing

### Test Suite

The implementation includes automated tests validating:

**Correctness Tests:**
- Basic insertion and lookup for known key sets
- Negative lookups (keys not in table return -1)
- Both algorithms produce identical lookup results

**Stress Tests:**
- Large datasets (1000+ keys)
- All inserted keys successfully retrieved
- No false positives or false negatives

**Edge Cases:**
- Single-key tables
- Two-key tables
- Keys with common prefixes
- Variable-length keys

**Expected output:**
```
=================================
Perfect Hashing - Testing
=================================

Running correctness test...
Basic Correctness Passed!

Running collision-free test...
Collision Test Passed!

Running stress test...
Stress Test Passed!

Running edge case tests...
Edge Cases Passed!

=================================
All Tests Passed!
=================================
```

### Test Validation

All tests validate the perfect hashing guarantee: **zero collisions** in final structure. Each test verifies that every inserted key can be retrieved with a single lookup operation.


## Performance Benchmarking

### Methodology

The benchmarking framework employs rigorous statistical methodology to ensure reliable performance measurements. Each configuration is tested across 10 independent trials, with 3 warmup runs executed beforehand. This is because I noticed that when conducting multiple trials, the first trials always took longer than ones that come after, which could be due to preloaded cache contents for newer trials. All timing measurements use `clock_gettime(CLOCK_MONOTONIC)` for nanosecond-precision.

The benchmark suite measures four primary metrics: build time (hash table construction from key insertion to completion), lookup latency (per-key retrieval time), memory footprint (exact byte-level accounting of all allocated structures), and build efficiency (the number of hash function generation attempts required). Statistical analysis computes the median, 95th percentile, 99th percentile, and standard deviation across all trials, providing both central tendency and tail behavior characterisation.

Test configurations span dataset sizes from 1,000 to 50,000 keys, using randomly generated strings of 50 characters composed of lowercase letters (a-z). Note that I would've gone to much larger dataset sizes, but my laptop couldn't handle it. Each key is unique within its dataset, verified through ```key_set_cleaner()```. This ensures the perfect hashing algorithms operate on valid input that matches real-world use cases (and to also avoid infinite loops we can find in ```build_second_level_bucketing()```). 

### Build Time Analysis

**Average Build Time (ms)**

| Hashing Type    | 1,000 | 2,500 | 5,000 | 7,500 | 10,000 | 25,000 | 50,000 |
| --------------- | ----- | ----- | ----- | ----- | ------ | ------ | ------ |
| Perfect Hashing | 0.269 | 0.705 | 1.33  | 2.13  | 2.94   | 7.84   | 17.81  |
| Min Perfect     | 0.601  | 1.70  | 3.35  | 5.04  | 6.82   | 18.17  | 42.69  |



### Lookup Performance

**Average Lookup Time (ns)**

| Hashing Type    | 1,000 | 2,500 | 5,000 | 7,500 | 10,000 | 25,000 | 50,000 |
| --------------- | ----- | ----- | ----- | ----- | ------ | ------ | ------ |
| Perfect Hashing | 48    | 53    | 61    | 70    | 73     | 92     | 136    |
| Min Perfect     | 52    | 62    | 78    | 84    | 102    | 132    | 202    |


### Memory Usage

**Average Memory Usage Per Key (bytes)**
| Hashing Type    | 1,000  | 2,500  | 5,000  | 7,500  | 10,000 | 25,000 | 50,000 |
| --------------- | ------ | ------ | ------ | ------ | ------ | ------ | ------ |
| Perfect Hashing | 123.64 | 123.47 | 123.11 | 123.77 | 123.06 | 123.2  | 123.22 |
| Min Perfect     | 116.08 | 115.24 | 115.3  | 115.13 | 115.43 | 115.15 | 115.05 |

**Median Memory Usage** 
| Hashing Type    | 1,000 | 2,500 | 5,000 | 7,500 | 10,000 | 25,000 | 50,000 |
| --------------- | ----- | ----- | ----- | ----- | ------ | ------ | ------ |
| Perfect Hashing | 0.12  | 0.29  | 0.59  | 0.89  | 1.17   | 2.94   | 5.88   |
| Min Perfect     | 0.11  | 0.27  | 0.55  | 0.82  | 1.1    | 2.75   | 5.49   |




### Build Efficiency
**Average Total Attempts To Build Perfect Hash Table**
| Hashing Type    | 1,000 | 2,500  | 5,000   | 7,500   | 10,000 | 25,000  | 50,000  |
| --------------- | ----- | ------ | ------- | ------- | ------ | ------- | ------- |
| Perfect Hashing | 447.8 | 1162.9 | 2273.7  | 3483.8  | 4658.6 | 11542.9 | 23047.3 |
| Min Perfect     | 1650  | 4127.6 | 12268.6 | 15562.2 | 40646  | 80657.8 |         |


### Cache Miss Rates

### Key Findings



## Running The Code

```bash
make 
./test_hashing_correctness
./benchmark [num_keys] [key_len]
# Example: benchmark with 10,000 keys of length 50
./benchmark 10000 50
```

The program for benchmarking outputs detailed statistics for both Regular Perfect Hashing and Minimal Perfect Hashing, including per-trial results and aggregate statistics across all measured metrics.