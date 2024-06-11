INCLUDE(CMakePrintHelpers)
INCLUDE(FetchContent)
CMAKE_MINIMUM_REQUIRED(VERSION 3.28)  # CMake version check
PROJECT(fhatos)
SET(CMAKE_CXX_STANDARD 20)
IF(NOT WIN32)
    STRING(ASCII 27 Esc)
    SET(.. "${Esc}[m")
    SET(ColourBold  "${Esc}[1m")
    SET(.r         "${Esc}[31m")
    SET(.g       "${Esc}[32m")
    SET(.y      "${Esc}[33m")
    SET(Blue        "${Esc}[34m")
    SET(Magenta     "${Esc}[35m")
    SET(Cyan        "${Esc}[36m")
    SET(White       "${Esc}[37m")
    SET(BoldRed     "${Esc}[1;31m")
    SET(BoldGreen   "${Esc}[1;32m")
    SET(BoldYellow  "${Esc}[1;33m")
    SET(BoldBlue    "${Esc}[1;34m")
    SET(BoldMagenta "${Esc}[1;35m")
    SET(BoldCyan    "${Esc}[1;36m")
    SET(BoldWhite   "${Esc}[1;37m")
ENDIF()


######## PROJECT LIBRARY
MESSAGE(CHECK_START "${.y}CMaking ${PROJECT_NAME} Library${..}")
INCLUDE_DIRECTORIES(src include)
# set(SOURCE_FILES src/main.cpp)
ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCE_FILES})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME ${PROJECT_NAME}
        SUFFIX ".out"
)
TARGET_SOURCES(${PROJECT_NAME} PRIVATE src/main.cpp)
TARGET_COMPILE_FEATURES(${PROJECT_NAME} PRIVATE cxx_std_20)
TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PRIVATE NATIVE)
CMAKE_PRINT_VARIABLES(PROJECT_NAME CMAKE_SOURCE_DIR CMAKE_INSTALL_PREFIX CMAKE_CXX_COMPILER_ID CMAKE_CXX_COMPILER_VERSION)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
####################################
######## EXTERNAL LIBRARIES ########
####################################
### ORDERED_MAP: INSERT ORDER MAP IMPLEMENTATION
MESSAGE(CHECK_START "${.y}Making Ordered Map Library${..}")
FETCHCONTENT_DECLARE(
        ordered_map
        GIT_REPOSITORY https://github.com/Tessil/ordered-map.git
        GIT_TAG v1.1.0
)
FETCHCONTENT_MAKEAVAILABLE(ordered_map)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/_deps/ordered_map-src/include/tsl/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/tsl)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/_deps/ordered_map-src/include/tsl/ DESTINATION ${CMAKE_SOURCE_DIR}/include/tsl)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE tsl::ordered_map)
MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
### LEXY: DSL PARSING
#FetchContent_Declare(lexy URL https://lexy.foonathan.net/download/lexy-src.zip)
#FetchContent_MakeAvailable(lexy)
#install(DIRECTORY ${CMAKE_SOURCE_DIR}/_deps/lexy-src/include/lexy/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/lexy)
#install(DIRECTORY ${CMAKE_SOURCE_DIR}/_deps/lexy-src/include/lexy/ DESTINATION ${CMAKE_SOURCE_DIR}/include/lexy)
#target_link_libraries(${PROJECT_NAME} PRIVATE foonathan::lexy)
####################################
####################################

####################################
############# TESTING ##############
####################################
ADD_SUBDIRECTORY(test)

####################################
########## DOCUMENTATION ###########
####################################
MESSAGE(CHECK_START "${.y}CMaking Doxygen Documentation${..}")
OPTION(BUILD_WITH_DOCS "Build documentation" ON)
IF (TASKSCH_BUILD_WITH_DOCS)
    ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/docs)
ENDIF ()

FIND_PACKAGE(Doxygen)
IF (DOXYGEN_FOUND)
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
ENDIF ()
MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")