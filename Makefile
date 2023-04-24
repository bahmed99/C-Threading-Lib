TEST_DIR=test
SRC_DIR=src
INSTALL_DIR=install
BIN_DIR=$(INSTALL_DIR)/bin
BUILD_DIR=build
B_LIB_DIR=$(BUILD_DIR)/libarg
BINS= 01-main 02-switch 03-equity 21-create-many 12-join-main 11-join 22-create-many-recursive 23-create-many-once 31-switch-many 32-switch-many-join 33-switch-many-cascade 51-fibonacci 71-preemption 61-mutex 62-mutex 91-merge-sort 92-insertion-sort
BINS_EXEC= "01-main" "02-switch" "03-equity" "11-join" "12-join-main" "21-create-many 20" "22-create-many-recursive 20" "23-create-many-once 20" "31-switch-many 10 20" "32-switch-many-join 10 20" "33-switch-many-cascade 20 5" "51-fibonacci 16" "61-mutex 20" "62-mutex 20" "71-preemption 3" "91-merge-sort 10" "92-insertion-sort 10"

VALGRIND_FLAGS=--leak-check=full --show-reachable=yes --track-origins=yes
MAKEFLAGS += --no-print-directory
CC=gcc
CCFLAGS=-O3
# CCFLAGS=-Wall

GREEN=\033[0;32m
RED=\033[0;31m
NC=\033[0m

all: install 

install: clean $(BINS)

pthreads: create_dirs 
	@for f in $(BINS) ; do \
		$(CC) $(CCFLAGS) -o $(BIN_DIR)/$$f $(TEST_DIR)/$$f.c -I$(SRC_DIR) -DUSE_PTHREAD ; \
	done
	
check: $(BINS)
	@for f in $(BINS_EXEC) ; do \
		echo "$(GREEN)START TEST: $$f$(NC)"; \
		$(BIN_DIR)/$$f ; \
		if [ 0 -eq "$$?" ]; then \
			echo "$(GREEN)➜ TEST COMPLETED: $$f$(NC)"; \
    	else \
			echo "$(RED)➜ TEST FAILED: $$f$(NC)"; \
    	fi ; \
	done

graph:
	@python3 display.py run_all

valgrind: install
	@for f in $(BINS_EXEC) ; do \
		echo "$(GREEN)START VALGRIND TEST: $$f$(NC)"; \
		valgrind $(VALGRIND_FLAGS) $(BIN_DIR)/$$f ; \
		if [ 0 -eq "$$?" ]; then \
			echo "$(GREEN)➜ VALGRIND TEST COMPLETED: $$f$(NC)"; \
    	else \
			echo "$(RED)➜ VALGRIND TEST FAILED: $$f$(NC)"; \
    	fi ; \
	done

slow_v_check: install
	@for f in $(BINS_EXEC) ; do \
		clear && echo "$(GREEN)START VALGRIND TEST: $$f$(NC)"; \
		valgrind $(VALGRIND_FLAGS) $(BIN_DIR)/$$f ; \
		if [ 0 -eq "$$?" ]; then \
			echo "$(GREEN)➜ VALGRIND TEST COMPLETED: $$f$(NC)"; \
    	else \
			echo "$(RED)➜ VALGRIND TEST FAILED: $$f$(NC)"; \
    	fi ; \
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
	@rm -rf $(INSTALL_DIR) $(BUILD_DIR) ; echo "Files Cleaned !"

.PHONY: create_dirs all install pthreads check graph valgrind slow_v_check