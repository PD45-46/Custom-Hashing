CC = cc
CFLAGS = -Wall -Wextra -O2 -g

SRC = src/hash.c
OBJ = $(SRC:.c=.o)

TEST = tests/test_ph1_correctness.c
TEST_BIN = test_ph1_correctness

all: $(TEST_BIN)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_BIN): $(TEST) $(OBJ)
	$(CC) $(CFLAGS) $(TEST) $(OBJ) -o $(TEST_BIN)

clean:
	rm -f $(OBJ) $(TEST_BIN)

.PHONY: all clean
