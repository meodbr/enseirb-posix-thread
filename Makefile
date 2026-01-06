# Compilateur et options
CC=gcc
CFLAGS=-Wall -Wextra -std=c99 

# Répertoires
SRC_DIR=src
TEST_DIR=tst
BUILD_DIR=build
TEST_BUILD_DIR=$(BUILD_DIR)/tst
LIB_BUILD_DIR=$(BUILD_DIR)/lib
INSTALL_DIR=install
INSTALL_BIN_DIR=$(INSTALL_DIR)/bin
INSTALL_LIB_DIR=$(INSTALL_DIR)/lib

# Fichiers
LIB_SRC=$(wildcard $(SRC_DIR)/*.c)
LIB_OBJ=$(LIB_SRC:$(SRC_DIR)/%.c=$(LIB_BUILD_DIR)/%.o)
LIB=$(LIB_BUILD_DIR)/libthread.so

TESTS = 01-main 02-switch 03-equity 11-join 12-join-main 21-create-many 22-create-many-recursive 23-create-many-once 31-switch-many 32-switch-many-join 33-switch-many-cascade 51-fibonacci 61-mutex 62-mutex 63-mutex-equity 64-mutex-join 71-preemption 81-deadlock 91-priority

TEST_SRC=$(addprefix $(TEST_DIR)/, $(addsuffix .c, $(TESTS)))
TEST_OBJ=$(TEST_SRC:$(TEST_DIR)/%.c=$(TEST_BUILD_DIR)/%.o)

PTHREAD_TEST_OBJ=$(TEST_SRC:$(TEST_DIR)/%.c=$(TEST_BUILD_DIR)/%-pthread.o)
TEST=$(TEST_OBJ:$(TEST_BUILD_DIR)/%.o=$(TEST_BUILD_DIR)/%)
PTHREAD_TEST=$(PTHREAD_TEST_OBJ:$(TEST_BUILD_DIR)/%.o=$(TEST_BUILD_DIR)/%)

# Règles
all: lib libpr tests

# Créer les répertoires de build s'ils n'existent pas
$(shell mkdir -p $(TEST_BUILD_DIR) $(LIB_BUILD_DIR))

# Compile la librairie partagée pour les threads
lib: $(LIB) $(LIB_OBJ)

$(LIB_BUILD_DIR)/%.so: $(LIB_BUILD_DIR)/%.o
	$(CC) -o $@ -shared -fPIC $^

$(LIB_BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -o $@ $(CFLAGS) -fPIC -c $< 

libpr: $(LIB_BUILD_DIR)/libthreadpr.so $(LIB_OBJ:.o=-pr.o)

$(LIB_BUILD_DIR)/libthreadpr.so: $(LIB_BUILD_DIR)/libthread-pr.o
	$(CC) -o $@ -shared -fPIC $^

$(LIB_BUILD_DIR)/%-pr.o: $(SRC_DIR)/%.c
	$(CC) -o $@ $(CFLAGS) -fPIC -c $< -DUSE_PREEMPTION 

# Compiler les tests pour les threads. Chaque test est compilé dans son propre exécutable sans -DUSE_THREAD
tests: $(TEST) $(TEST_OBJ)

$(TEST_BUILD_DIR)/%: $(TEST_BUILD_DIR)/%.o $(LIB) $(LIB_BUILD_DIR)/libthreadpr.so
	$(CC) -o $@ $(CFLAGS) $< -L$(LIB_BUILD_DIR) -lthread -Wl,-rpath=$(INSTALL_LIB_DIR)

$(TEST_BUILD_DIR)/71-preemption: $(TEST_BUILD_DIR)/71-preemption.o $(LIB_BUILD_DIR)/libthreadpr.so
	$(CC) -o $@ $(CFLAGS) $< -L$(LIB_BUILD_DIR) -lthreadpr -Wl,-rpath=$(INSTALL_LIB_DIR)

$(TEST_BUILD_DIR)/62-mutex: $(TEST_BUILD_DIR)/62-mutex.o $(LIB_BUILD_DIR)/libthreadpr.so
	$(CC) -o $@ $(CFLAGS) $< -L$(LIB_BUILD_DIR) -lthreadpr -Wl,-rpath=$(INSTALL_LIB_DIR)

$(TEST_BUILD_DIR)/%.o: $(TEST_DIR)/%.c
	$(CC) -o $@ $(CFLAGS) -c $< -I $(SRC_DIR)

# Compiler les tests pour les pthreads. Chaque test est compilé dans son propre exécutable avec -DUSE_PTHREAD
pthreads: $(PTHREAD_TEST) $(PTHREAD_TEST_OBJ)

$(TEST_BUILD_DIR)/%-pthread: $(TEST_BUILD_DIR)/%-pthread.o
	$(CC) -o $@ $(CFLAGS) $< -lpthread -I $(SRC_DIR)

$(TEST_BUILD_DIR)/%-pthread.o: $(TEST_DIR)/%.c
	$(CC) -o $@ $(CFLAGS) -DUSE_PTHREAD -I $(SRC_DIR) -c $< 

# Installation des fichiers cibles dans le répertoire install
install: lib tests pthreads
	cp $(LIB) $(INSTALL_LIB_DIR)
	cp $(LIB_BUILD_DIR)/libthreadpr.so $(INSTALL_LIB_DIR)
	cp $(TEST) $(INSTALL_BIN_DIR)
	cp $(PTHREAD_TEST) $(INSTALL_BIN_DIR)

# Exécution des tests
check: install
	./run_tests.sh

# Valgrind
valgrind: install
	./run_tests.sh -v

graphs: install
	./run_tests.sh -g

# Suppression du répertoire build et des fichier installés
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(INSTALL_LIB_DIR)/*
	rm -f $(INSTALL_BIN_DIR)/*

.PHONY: all lib tests pthreads install check clean