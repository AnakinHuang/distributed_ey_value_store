CXX      := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -pthread -I./src

# Paths
SRC_DIR := src
TEST_DIR := tests

# Source files
NODE_SRCS := $(SRC_DIR)/node.cpp $(SRC_DIR)/network.cpp
CLIENT_SRCS := $(SRC_DIR)/client.cpp $(SRC_DIR)/network.cpp

# Executables
EXES := node client test_lamport test_kv_store

# Default target
all: node client tests

# Build node (replica)
node: $(NODE_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Build client
client: $(CLIENT_SRCS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Unit tests (header-only dependencies)
test_lamport:
	$(CXX) $(CXXFLAGS) $(TEST_DIR)/test_lamport.cpp -o $@

test_kv_store:
	$(CXX) $(CXXFLAGS) $(TEST_DIR)/test_kv_store.cpp -o $@

.PHONY: tests
tests: test_lamport test_kv_store

.PHONY: clean
clean:
	rm -f $(EXES)
