include(CMakePrintHelpers)
include(FetchContent)
cmake_minimum_required(VERSION 3.29)  # CMake version check
project(fhatos)
set(CMAKE_CXX_STANDARD 20)

######## PROJECT LIBRARY
include_directories(src include)
# set(SOURCE_FILES src/main.cpp)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES
        OUTPUT_NAME ${PROJECT_NAME}
        SUFFIX ".out"
)
target_sources(${PROJECT_NAME} PRIVATE src/main.cpp)
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
install(DIRECTORY ${CMAKE_SOURCE_DIR}/_deps/ordered_map-src/include/tsl/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/tsl)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/_deps/ordered_map-src/include/tsl/ DESTINATION ${CMAKE_SOURCE_DIR}/include/tsl)
target_link_libraries(${PROJECT_NAME} PRIVATE tsl::ordered_map)
### LEXY: DSL PARSING
#FetchContent_Declare(lexy URL https://lexy.foonathan.net/download/lexy-src.zip)
#FetchContent_MakeAvailable(lexy)
#install(DIRECTORY ${CMAKE_SOURCE_DIR}/_deps/lexy-src/include/lexy/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/lexy)
#install(DIRECTORY ${CMAKE_SOURCE_DIR}/_deps/lexy-src/include/lexy/ DESTINATION ${CMAKE_SOURCE_DIR}/include/lexy)
#target_link_libraries(${PROJECT_NAME} PRIVATE foonathan::lexy)
####################################
####################################

#####

#project(fhatos-test)
#add_library(fhatos-test STATIC test/language/test_binary_obj/test_binary_obj.cpp)
#target_include_directories(fhatos-test PRIVATE test)
#set_target_properties(fhatos-test PROPERTIES UNITY_BUILD ON UNITY_BUILD_MODE BATCH UNITY_BUILD_BATCH_SIZE 16)
#enable_testing()

####################################
########## DOCUMENTATION ###########
####################################
OPTION(BUILD_WITH_DOCS "Build documentation" ON)
IF(TASKSCH_BUILD_WITH_DOCS)
    ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/docs)
ENDIF()

FIND_PACKAGE( Doxygen )
IF(DOXYGEN_FOUND)
    MESSAGE(STATUS "Doxygen found: ${DOXYGEN_EXECUTABLE} -- ${DOXYGEN_VERSION}")
    # Set Doxygen input and output files.
    SET(DOXYGEN_INPUT_DIR ${PROJECT_SOURCE_DIR})
    SET(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
    SET(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/xml/index.xml)
    SET(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    SET(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    # Generate DoxyFile from the input file.
    CONFIGURE_FILE(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)
    # Create Output directory.
    FILE(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})
    # Command for generating doc from Doxygen config file.
    ADD_CUSTOM_COMMAND(OUTPUT ${DOXYGEN_INDEX_FILE}
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
            MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
            COMMENT "Generating Doxygen Documentation"
            VERBATIM)
    # Create CMake Target for generating doc.
    ADD_CUSTOM_TARGET(docs ALL DEPENDS ${DOXYGEN_INDEX_FILE})
ENDIF()