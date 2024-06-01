include(CMakePrintHelpers)
include(FetchContent)
cmake_minimum_required(VERSION 3.29)  # CMake version check
project(fhatos)
set(CMAKE_CXX_STANDARD 20)

######## PROJECT LIBRARY
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
include_directories(${CMAKE_SOURCE_DIR}/include)

####################################
######## EXTERNAL LIBRARIES ########
####################################
### ORDERED_MAP: INSERT ORDER MAP IMPLEMENTATION
FetchContent_Declare(
        ordered_map
        GIT_REPOSITORY https://github.com/Tessil/ordered-map.git
        GIT_TAG v1.1.0
)
FetchContent_MakeAvailable(ordered_map)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/_deps/ordered_map-src/include/tsl/ DESTINATION ${CMAKE_SOURCE_DIR}/include/tsl)
target_link_libraries(${PROJECT_NAME} PUBLIC tsl::ordered_map)
### LEXY: DSL PARSING
FetchContent_Declare(lexy URL https://lexy.foonathan.net/download/lexy-src.zip)
FetchContent_MakeAvailable(lexy)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/_deps/lexy-src/include/lexy/ DESTINATION ${CMAKE_SOURCE_DIR}/include/lexy)
target_link_libraries(${PROJECT_NAME} PUBLIC foonathan::lexy)
####################################
####################################

#####

#project(fhatos-test)
#add_library(fhatos-test STATIC test/language/test_binary_obj/test_binary_obj.cpp)
#target_include_directories(fhatos-test PRIVATE test)
#set_target_properties(fhatos-test PROPERTIES UNITY_BUILD ON UNITY_BUILD_MODE BATCH UNITY_BUILD_BATCH_SIZE 16)
#enable_testing()