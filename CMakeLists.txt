cmake_minimum_required(VERSION 3.31)
project(csc458_final_project)

set(CMAKE_CXX_STANDARD 20)

add_executable(csc458_final_project
        src/node.cpp
        src/client.cpp
        src/message.hpp
        src/lamport.hpp
        src/kv_store.hpp
        tests/test_kv_store.cpp
)
