project(craftworld)

cmake_minimum_required (VERSION 3.12)

# Set flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Testing
enable_testing()

set(CRAFTWORLD_SOURCES
    src/definitions.h
    src/craftworld_base.cpp 
    src/craftworld_base.h 
    src/util.cpp 
    src/util.h
)

# Build library
add_library(craftworld STATIC ${CRAFTWORLD_SOURCES})
target_compile_options(craftworld PRIVATE 
    -Wall -Wextra 
    $<$<CONFIG:RELEASE>:-O3> $<$<CONFIG:RELEASE>:-DNDEBUG>
    $<$<CONFIG:DEBUG>:-g> $<$<CONFIG:DEBUG>:-DDEBUG> $<$<CONFIG:DEBUG>:-pg>
)

target_include_directories(craftworld PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# Build child targets
add_subdirectory(test)
