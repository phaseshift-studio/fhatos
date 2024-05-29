include(CMakePrintHelpers)
cmake_minimum_required(VERSION 3.13)  # CMake version check
project(fhatos)
set(CMAKE_CXX_STANDARD 20)

# Add main.cpp file of the project root directory as a source file
include_directories(src)
set(SOURCE_FILES src/main.cpp)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES
        OUTPUT_NAME ${PROJECT_NAME}
        SUFFIX ".out"
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_compile_definitions(${PROJECT_NAME} PRIVATE NATIVE)
cmake_print_variables(CMAKE_CXX_COMPILER_ID)
cmake_print_variables(CMAKE_CXX_COMPILER_VERSION)

