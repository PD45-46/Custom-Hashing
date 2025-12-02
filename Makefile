CC = cc
CFLAGS = -Wall -Wextra -O2 -g -Iinclude

# Core library sources
SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

# Benchmark library sources (but not benchmark.c)
BENCH_LIB_SRC = $(filter-out benchmarks/benchmark.c, $(wildcard benchmarks/*.c))
BENCH_LIB_OBJ = $(BENCH_LIB_SRC:.c=.o)

# Test target
TEST = tests/test_hashing_correctness.c
TEST_BIN = test_hashing_correctness

# Benchmark target
BENCH = benchmarks/benchmark.c
BENCH_BIN = benchmark

all: $(TEST_BIN) $(BENCH_BIN)

# Generic object rule
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build test binary (only core library)
$(TEST_BIN): $(TEST) $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

# Build benchmark binary (core lib + benchmark lib)
$(BENCH_BIN): $(BENCH) $(OBJ) $(BENCH_LIB_OBJ)
	$(CC) $(CFLAGS) $^ -lm -o $@

clean:
	rm -f $(OBJ) $(BENCH_LIB_OBJ) $(TEST_BIN) $(BENCH_BIN)

.PHONY: all clean
