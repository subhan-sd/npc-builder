cmake_minimum_required(VERSION 3.20)
project(npc_builder LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Core Static Library
add_library(npc_core STATIC
    core/src/generator.cpp
)

target_include_directories(npc_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/core/include
)

target_compile_options(npc_core PRIVATE -Wall -Wextra -Wpedantic)

# C-API shared bridge for Godot or other engines
add_library(npc_builder SHARED
    core/src/c_api.cpp
)

target_link_libraries(npc_builder PRIVATE npc_core)
target_include_directories(npc_builder PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/core/include
)

# Test Executable
add_executable(npc_test
    tests/test_generator.cpp
)

target_link_libraries(npc_test PRIVATE npc_core)
