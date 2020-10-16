.PHONY = tests
SRC_DIR = src
HEAD_DIR = headers
BUILD_DIR = build
INSTALL_DIR = install
TEST_DIR = tests
CUR_DIR = $(shell pwd)
TEST_PREF = test_

CC := gcc
CFLAGS := -Wall -lpthread

TESTS = 01-main        \
    02-switch    \
    11-join        \
    12-join-main \
    21-create-many \
    22-create-many-recursive \
    23-create-many-once \
    31-switch-many \
    32-switch-many-join \
	33-switch-many-cascade \
    51-fibonacci \
	61-mutex \
	62-mutex \
    contextes \
    example

TESTS_SRC := $(wildcard $(TEST_DIR)/*.c)
TESTS_NAME := $(patsubst %,$(TEST_PREF)%,$(TESTS))

tests: libthread $(TESTS_NAME)


libthread:
	cd $(BUILD_DIR) && $(CC) $(CFLAGS) -fPIC -c ../$(SRC_DIR)/thread.c
	$(CC) -shared -D_XOPEN_SOURCE -o $(BUILD_DIR)/libthread.so $(BUILD_DIR)/thread.o
	cd $(BUILD_DIR) && $(CC) $(CFLAGS) -std=c99 -I ../headers/ -c ../tests/trifusion.c -lthread -L $(CUR_DIR)/$(BUILD_DIR)
	cd $(BUILD_DIR) && $(CC) $(CFLAGS) -std=c99 -o trifusion trifusion.o -lthread -L $(CUR_DIR)/$(BUILD_DIR)
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR)

$(TEST_PREF)%:
	cd $(BUILD_DIR) && $(CC) $(CFLAGS) -I ../headers/ -c ../$(patsubst $(TEST_PREF)%,$(TEST_DIR)/%.c,$@) -lthread -L $(CUR_DIR)/$(BUILD_DIR)
	cd $(BUILD_DIR) && $(CC) $(CFLAGS) -o $(patsubst $(TEST_PREF)%,%,$@) $(patsubst $(TEST_PREF)%,%.o,$@) -lthread -L $(CUR_DIR)/$(BUILD_DIR)

check: tests
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) ./$(BUILD_DIR)/01-main
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) ./$(BUILD_DIR)/02-switch
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) ./$(BUILD_DIR)/11-join
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) ./$(BUILD_DIR)/12-join-main
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) ./$(BUILD_DIR)/21-create-many 4
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) ./$(BUILD_DIR)/22-create-many-recursive 4
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) ./$(BUILD_DIR)/23-create-many-once 4
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) ./$(BUILD_DIR)/31-switch-many 4 6
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) ./$(BUILD_DIR)/33-switch-many-cascade 10 15
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) ./$(BUILD_DIR)/32-switch-many-join 4 6
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) ./$(BUILD_DIR)/51-fibonacci 10
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) ./$(BUILD_DIR)/61-mutex 4
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) ./$(BUILD_DIR)/62-mutex 4
	#LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) ./$(BUILD_DIR)/contextes
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) ./$(BUILD_DIR)/example
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) ./$(BUILD_DIR)/trifusion 20



valgrind: tests
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) valgrind ./$(BUILD_DIR)/01-main
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) valgrind ./$(BUILD_DIR)/02-switch
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) valgrind ./$(BUILD_DIR)/11-join
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) valgrind ./$(BUILD_DIR)/12-join-main
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) valgrind ./$(BUILD_DIR)/21-create-many 4
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) valgrind ./$(BUILD_DIR)/22-create-many-recursive 4
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) valgrind ./$(BUILD_DIR)/23-create-many-once 4
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) valgrind ./$(BUILD_DIR)/31-switch-many 4 6
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) valgrind ./$(BUILD_DIR)/32-switch-many-join 4 6
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) valgrind ./$(BUILD_DIR)/33-switch-many-cascade 10 15
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) valgrind ./$(BUILD_DIR)/51-fibonacci 10
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) valgrind ./$(BUILD_DIR)/61-mutex 4
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) valgrind ./$(BUILD_DIR)/62-mutex 4
	#LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) valgrind ./$(BUILD_DIR)/contextes
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) valgrind ./$(BUILD_DIR)/example
	LD_LIBRARY_PATH=$(CUR_DIR)/$(BUILD_DIR) valgrind ./$(BUILD_DIR)/trifusion 20

pthreads: tests
	$(CC) $(TEST_DIR)/01-main.c -DUSE_PTHREAD -lpthread -I $(HEAD_DIR)/ -o $(BUILD_DIR)/01-main_with_pthreads
	$(CC) $(TEST_DIR)/02-switch.c -DUSE_PTHREAD -lpthread -I $(HEAD_DIR)/ -o $(BUILD_DIR)/02-switch_with_pthreads
	$(CC) $(TEST_DIR)/11-join.c -DUSE_PTHREAD -lpthread -I $(HEAD_DIR)/ -o $(BUILD_DIR)/11-join_with_pthreads
	$(CC) $(TEST_DIR)/12-join-main.c -DUSE_PTHREAD -lpthread -I $(HEAD_DIR)/ -o $(BUILD_DIR)/12-join-main_with_pthreads
	$(CC) $(TEST_DIR)/21-create-many.c -DUSE_PTHREAD -lpthread -I $(HEAD_DIR)/ -o $(BUILD_DIR)/21-create-many_with_pthreads
	$(CC) $(TEST_DIR)/22-create-many-recursive.c -DUSE_PTHREAD -lpthread -I $(HEAD_DIR)/ -o $(BUILD_DIR)/22-create-many-recursive_with_pthreads
	$(CC) $(TEST_DIR)/23-create-many-once.c -DUSE_PTHREAD -lpthread -I $(HEAD_DIR)/ -o $(BUILD_DIR)/23-create-many-once_with_pthreads
	$(CC) $(TEST_DIR)/31-switch-many.c -DUSE_PTHREAD -lpthread -I $(HEAD_DIR)/ -o $(BUILD_DIR)/31-switch-many_with_pthreads
	$(CC) $(TEST_DIR)/32-switch-many-join.c -DUSE_PTHREAD -lpthread -I $(HEAD_DIR)/ -o $(BUILD_DIR)/32-switch-many-join_with_pthreads
	$(CC) $(TEST_DIR)/33-switch-many-cascade.c -DUSE_PTHREAD -lpthread -I $(HEAD_DIR)/ -o $(BUILD_DIR)/33-switch-many-cascade_with_pthreads
	$(CC) $(TEST_DIR)/51-fibonacci.c -DUSE_PTHREAD -lpthread -I $(HEAD_DIR)/ -o $(BUILD_DIR)/51-fibonacci_with_pthreads
	$(CC) $(TEST_DIR)/61-mutex.c -DUSE_PTHREAD -lpthread -I $(HEAD_DIR)/ -o $(BUILD_DIR)/61-mutex_with_pthreads
	$(CC) $(TEST_DIR)/62-mutex.c -DUSE_PTHREAD -lpthread -I $(HEAD_DIR)/ -o $(BUILD_DIR)/62-mutex_with_pthreads
	$(CC) $(TEST_DIR)/example.c -DUSE_PTHREAD -lpthread -I $(HEAD_DIR)/ -o $(BUILD_DIR)/example_with_pthreads
	$(CC) $(TEST_DIR)/trifusion.c -DUSE_PTHREAD -lpthread -I $(HEAD_DIR)/ -o $(BUILD_DIR)/trifusion_with_pthreads
	./$(BUILD_DIR)/01-main_with_pthreads
	./$(BUILD_DIR)/02-switch_with_pthreads
	./$(BUILD_DIR)/11-join_with_pthreads
	./$(BUILD_DIR)/12-join-main_with_pthreads
	./$(BUILD_DIR)/21-create-many_with_pthreads 4
	./$(BUILD_DIR)/22-create-many-recursive_with_pthreads 4
	./$(BUILD_DIR)/23-create-many-once_with_pthreads 4
	./$(BUILD_DIR)/31-switch-many_with_pthreads 4 6
	./$(BUILD_DIR)/32-switch-many-join_with_pthreads 4 6
	./$(BUILD_DIR)/33-switch-many-cascade_with_pthreads 10 15
	./$(BUILD_DIR)/51-fibonacci_with_pthreads 10 # plante à partir de 22
	./$(BUILD_DIR)/61-mutex_with_pthreads 4
	./$(BUILD_DIR)/62-mutex_with_pthreads 4
	./$(BUILD_DIR)/example_with_pthreads
	./$(BUILD_DIR)/trifusion_with_pthreads 20

graphs : check pthreads
	python3 $(TEST_DIR)/graphs.py build/51-fibonacci
	python3 $(TEST_DIR)/graphs.py build/trifusion build/trifusion_with_pthreads
	python3 $(TEST_DIR)/graphs.py build/62-mutex build/62-mutex_with_pthreads

copy_tests:
	for execut in $(TESTS); do \
		cp $(BUILD_DIR)/$$execut $(INSTALL_DIR)/bin/; \
	done

install:
	rm -rf $(INSTALL_DIR)
	mkdir $(INSTALL_DIR)
	mkdir $(INSTALL_DIR)/lib
	mkdir $(INSTALL_DIR)/bin
	cp $(BUILD_DIR)/libthread.so $(INSTALL_DIR)/lib
	make copy_tests
	ls $(INSTALL_DIR)/bin
	ls $(INSTALL_DIR)/lib


clean:
	rm -rf $(INSTALL_DIR)
	rm -rf $(BUILD_DIR)/*
	touch $(BUILD_DIR)/gotogit
	rm -f *.o
