cmake_minimum_required(VERSION 3.20)
project(npc_builder LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warnings and visibility
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# Core Static Library
add_library(npc_core STATIC
    core/src/generator.cpp
)

set_target_properties(npc_core PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(npc_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/core/include
)

# C-API shared library
add_library(npc_builder SHARED
    core/src/c_api.cpp
)

target_link_libraries(npc_builder PRIVATE npc_core)
target_include_directories(npc_builder PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/core/include
)

# Testing setup
enable_testing()

add_executable(npc_test
    tests/test_generator.cpp
)
target_link_libraries(npc_test PRIVATE npc_core)
target_include_directories(npc_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
add_test(NAME CoreUnitTests COMMAND npc_test)

# Golden Fixture Generator
add_executable(generate_fixtures
    tests/generate_fixtures.cpp
)
target_link_libraries(generate_fixtures PRIVATE npc_core)
target_include_directories(generate_fixtures PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# Golden Fixture Test
add_executable(test_golden
    tests/test_golden.cpp
)
target_link_libraries(test_golden PRIVATE npc_core)
target_include_directories(test_golden PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
add_test(NAME GoldenFixtureTest COMMAND test_golden ${CMAKE_CURRENT_SOURCE_DIR}/tests/golden/matriks-acuan.txt)

# C API Parity & Verification Test
add_executable(test_c_api
    tests/test_c_api.cpp
)
target_link_libraries(test_c_api PRIVATE npc_core npc_builder)
target_include_directories(test_c_api PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
add_test(NAME CApiParityTest COMMAND test_c_api)

# Serializer Verification Test
add_executable(test_serializer
    tests/test_serializer.cpp
)
target_link_libraries(test_serializer PRIVATE npc_core)
target_include_directories(test_serializer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)
add_test(NAME SerializerVerificationTest COMMAND test_serializer)

# Benchmark Utility
add_executable(benchmark_render
    tests/benchmark_render.cpp
)
target_link_libraries(benchmark_render PRIVATE npc_core)
target_include_directories(benchmark_render PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)

# GDExtension target (optional toggle if godot-cpp is present)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/extension/CMakeLists.txt")
    add_subdirectory(extension)
endif()
