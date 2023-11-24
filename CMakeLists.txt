cmake_minimum_required (VERSION 3.12)

project(craftworld)

# Sources
set(CRAFTWORLD_SOURCES
    src/definitions.h
    src/craftworld_base.cpp 
    src/craftworld_base.h 
    src/util.cpp 
    src/util.h
)

# Build library
add_library(craftworld STATIC ${CRAFTWORLD_SOURCES})
target_compile_features(craftworld PUBLIC cxx_std_17)
target_include_directories(craftworld PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# Build tests
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    option(BUILD_TESTS "Build the unit tests" OFF)
    if (${BUILD_TESTS})
        enable_testing()
        add_subdirectory(test)
    endif()
endif()
