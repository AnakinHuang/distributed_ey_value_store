CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -pthread -Isrc
SRC := $(wildcard src/*.cpp)
OBJ := $(SRC:src/%.cpp=build/%.o)
TESTS := tests/test_node tests/test_client

all: client node

client: src/client.cpp $(OBJ)
	$(CXX) $(CXXFLAGS) $< -o $@

node: src/node.cpp $(OBJ)
	$(CXX) $(CXXFLAGS) $< -o $@

build/%.o: src/%.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

tests: $(TESTS)

tests/%: tests/%.cpp node client
	$(CXX) $(CXXFLAGS) $< ../node ../client -o $@

.PHONY: clean
clean:
	rm -rf build node client $(TESTS)
