TEST_DIR=test
SRC_DIR=src
INSTALL_DIR=install
LIB_DIR=$(INSTALL_DIR)/lib
BIN_DIR=$(INSTALL_DIR)/bin
BUILD_DIR=build
B_LIB_DIR=$(BUILD_DIR)/lib
BINS= example 01-main 02-switch 03-equity 21-create-many 12-join-main 11-join 22-create-many-recursive 23-create-many-once 31-switch-many 32-switch-many-join 33-switch-many-cascade 61-mutex 62-mutex 51-fibonacci 81-deadlock
FIBONACCI_EXEC=51-fibonacci

VALGRIND_FLAGS=--leak-check=full --show-reachable=yes --track-origins=yes
MAKEFLAGS += --no-print-directory
CC=gcc

THREAD_NUMBER=100
YIELD_NUMBER=20
FIBONACCI=25


all: install pthreads 

install: $(BINS)

pthreads:
	@echo pthreads

check: $(BINS)
	@for f in $(BINS) ; do \
		echo "Running test: $$f"; \
		if [ $$f = $(FIBONACCI_EXEC) ]; then \
			$(BIN_DIR)/$$f $(FIBONACCI) ; \
		else \
			$(BIN_DIR)/$$f $(THREAD_NUMBER) $(YIELD_NUMBER) ; \
		fi ; \
	done

graph:
	@echo "graph"

valgrind: install
	@for f in $(BINS) ; do \
	echo "Running valgrind on $$f"; \
		if [ $$f = $(FIBONACCI_EXEC) ]; then \
			valgrind $(VALGRIND_FLAGS) $(BIN_DIR)/$$f $(FIBONACCI) ; \
		else \
			valgrind $(VALGRIND_FLAGS) $(BIN_DIR)/$$f $(THREAD_NUMBER) $(YIELD_NUMBER) ; \
		fi ; \
	done

$(B_LIB_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -o $@ -c $< -I$(SRC_DIR) -DUSE_PTHREAD

$(BINS): $(LIB_DIR)/libthread.a create_dirs
	$(CC) -o $(BIN_DIR)/$@ $(TEST_DIR)/$@.c -I$(SRC_DIR) -L$(LIB_DIR) -lthread

$(LIB_DIR)/libthread.a: $(B_LIB_DIR)/thread.o create_dirs
	ar rcs $@ $< 

create_dirs:
	@mkdir -p $(LIB_DIR) $(BIN_DIR) $(B_LIB_DIR)

clean:
	@rm -rf $(INSTALL_DIR)

# tests: test/*
# 	@for f in $^ ; do \
# 		out=$$(echo $$f | sed 's/\.c//g' | sed 's-test/-build/-g') ; make $$out ; \
# 	done

# build/%: test/%.c build
# 	gcc -DUSE_PTHREAD -I . -o $@ $<

# build:
# 	@mkdir -p build

.PHONY: tests