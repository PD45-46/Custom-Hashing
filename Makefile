CC = cc
CFLAGS = -Wall -Wextra -O2 -g -Iinclude

SRC = src/hash.c
OBJ = $(SRC:.c=.o)

# === Test targets ===
TEST = tests/test_ph1_correctness.c
TEST_BIN = test_ph1_correctness

# === Benchmark targets ===
BENCH = benchmarks/benchmark.c
BENCH_BIN = benchmark

all: $(TEST_BIN) $(BENCH_BIN)

# Object file rule
$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $< -o $@

# Build test binary
$(TEST_BIN): $(TEST) $(OBJ)
	$(CC) $(CFLAGS) $(TEST) $(OBJ) -o $(TEST_BIN)

# Build benchmark binary
$(BENCH_BIN): $(BENCH) $(OBJ)
	$(CC) $(CFLAGS) $(BENCH) $(OBJ) -o $(BENCH_BIN)

clean:
	rm -f $(OBJ) $(TEST_BIN) $(BENCH_BIN)

.PHONY: all clean
