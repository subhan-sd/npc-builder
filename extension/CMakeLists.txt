cmake_minimum_required(VERSION 3.20)
project(npc_builder_extension LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Include godot-cpp
add_subdirectory(godot-cpp)

add_library(npc_builder_extension SHARED
    src/register_types.cpp
    src/npc_generator.cpp
)

target_include_directories(npc_builder_extension PRIVATE
    src
    ${CMAKE_CURRENT_SOURCE_DIR}/../core/include
)

target_link_libraries(npc_builder_extension PRIVATE
    godot::cpp
    npc_core
)

# Output directory into godot/bin (ensuring direct placement across single & multi-config generators)
set_target_properties(npc_builder_extension PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../godot/bin
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../godot/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../godot/bin
    LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../godot/bin
    LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../godot/bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/../godot/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/../godot/bin
)
