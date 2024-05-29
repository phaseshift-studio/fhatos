include(CMakePrintHelpers)
cmake_minimum_required(VERSION 3.13)  # CMake version check
project(fhatos)
set(CMAKE_CXX_STANDARD 20)

# Add main.cpp file of the project root directory as a source file
include_directories(src)
set(SOURCE_FILES src/main.cpp)
add_executable(fhatos ${SOURCE_FILES})

target_compile_definitions (fhatos PRIVATE NATIVE)
cmake_print_variables(CMAKE_CXX_COMPILER_ID)
cmake_print_variables(CMAKE_CXX_COMPILER_VERSION)
cmake_print_variables(NATIVE)

# Add executable target with source files listed in SOURCE_FILES variable

