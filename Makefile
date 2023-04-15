TEST_DIR=test
SRC_DIR=src
INSTALL_DIR=install
BIN_DIR=$(INSTALL_DIR)/bin
BUILD_DIR=build
B_LIB_DIR=$(BUILD_DIR)/lib
BINS= example 01-main 02-switch 03-equity 21-create-many 12-join-main 11-join 22-create-many-recursive 23-create-many-once 31-switch-many 32-switch-many-join 33-switch-many-cascade 51-fibonacci 71-preemption 61-mutex 62-mutex 81-deadlock
BINS_EXEC= "01-main" "02-switch" "03-equity" "11-join" "12-join-main" "21-create-many 20" "22-create-many-recursive 20" "23-create-many-once 20" "31-switch-many 10 20" "32-switch-many-join 10 20" "33-switch-many-cascade 20 5" "51-fibonacci 12" "71-preemption 5"
# FIBONACCI_EXEC=51-fibonacci

VALGRIND_FLAGS=--leak-check=full --show-reachable=yes --track-origins=yes
MAKEFLAGS += --no-print-directory
CC=gcc
CCFLAGS=-Wall

# THREAD_NUMBER=100
# YIELD_NUMBER=20
# FIBONACCI=8

GREEN=\033[0;32m
NC=\033[0m


all: install pthreads 

install: $(BINS)

pthreads:
	@echo pthreads

check: $(BINS)
	# @for f in $(BINS) ; do \
	# 	echo "Running test: $$f"; \
	# 	if [ $$f = $(FIBONACCI_EXEC) ]; then \
	# 		$(BIN_DIR)/$$f $(FIBONACCI) ; \
	# 	else \
	# 		$(BIN_DIR)/$$f $(THREAD_NUMBER) $(YIELD_NUMBER) ; \
	# 	fi ; \
	# done
	@for f in $(BINS_EXEC) ; do \
		echo "$(GREEN)START TEST: $$f$(NC)"; \
		$(BIN_DIR)/$$f ; \
		echo "$(GREEN)➜ TEST COMPLETED: $$f$(NC)"; \
	done
	

graph:
	@echo "graph"

valgrind: install
	# @for f in $(BINS) ; do \
	# echo "Running valgrind on $$f"; \
	# 	if [ $$f = $(FIBONACCI_EXEC) ]; then \
	# 		valgrind $(VALGRIND_FLAGS) $(BIN_DIR)/$$f $(FIBONACCI) ; \
	# 	else \
	# 		valgrind $(VALGRIND_FLAGS) $(BIN_DIR)/$$f $(THREAD_NUMBER) $(YIELD_NUMBER) ; \
	# 	fi ; \
	# done
	@for f in $(BINS_EXEC) ; do \
		echo "$(GREEN)START VALGRIND TEST: $$f$(NC)"; \
		valgrind $(VALGRIND_FLAGS) $(BIN_DIR)/$$f ; \
		echo "$(GREEN)➜ VALGRIND TEST COMPLETED: $$f$(NC)"; \
	done

slow_v_check: install
	@for f in $(BINS_EXEC) ; do \
		clear && echo "$(GREEN)START VALGRIND TEST: $$f$(NC)"; \
		valgrind $(VALGRIND_FLAGS) $(BIN_DIR)/$$f ; \
		echo "$(GREEN)➜ VALGRIND TEST COMPLETED: $$f$(NC)"; \
		read pause ; \
	done

$(B_LIB_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CCFLAGS) -o $@ -c $< -I$(SRC_DIR)
# $(CC) -o $@ -c $< -I$(SRC_DIR) -DUSE_PTHREAD

$(BINS): create_dirs $(B_LIB_DIR)/libthread.a
	$(CC) $(CCFLAGS) -o $(BIN_DIR)/$@ $(TEST_DIR)/$@.c -I$(SRC_DIR) -L$(B_LIB_DIR) -lthread

$(B_LIB_DIR)/libthread.a: $(B_LIB_DIR)/thread.o $(B_LIB_DIR)/queue.o
	ar rcs $@ $^ 

create_dirs:
	@mkdir -p $(BIN_DIR) $(B_LIB_DIR)

clean:
	@rm -rf $(INSTALL_DIR) $(BUILD_DIR)

# tests: test/*
# 	@for f in $^ ; do \
# 		out=$$(echo $$f | sed 's/\.c//g' | sed 's-test/-build/-g') ; make $$out ; \
# 	done

# build/%: test/%.c build
# 	gcc -DUSE_PTHREAD -I . -o $@ $<

# build:
# 	@mkdir -p build

.PHONY: tests