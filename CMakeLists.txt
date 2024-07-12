INCLUDE(CMakePrintHelpers)
INCLUDE(FetchContent)
CMAKE_MINIMUM_REQUIRED(VERSION 3.28)  # CMake version check
PROJECT(fhatos)
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
IF (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    MESSAGE(FATAL_ERROR "In-source builds not allowed. Create a fhatos/build directory and run CMake from there (cmake ..).")
endif ()
CMAKE_PRINT_VARIABLES(PROJECT_NAME CMAKE_SOURCE_DIR CMAKE_BINARY_DIR CMAKE_INSTALL_PREFIX CMAKE_CXX_COMPILER_ID CMAKE_CXX_COMPILER_VERSION)
IF (NOT WIN32)
    STRING(ASCII 27 Esc)
    SET(.. "${Esc}[m")
    SET(ColourBold "${Esc}[1m")
    SET(.r "${Esc}[31m")
    SET(.g "${Esc}[32m")
    SET(.y "${Esc}[33m")
    SET(.b "${Esc}[34m")
    SET(.m "${Esc}[35m")
    SET(.c "${Esc}[36m")
    SET(.w "${Esc}[37m")
    SET(BoldRed "${Esc}[1;31m")
    SET(BoldGreen "${Esc}[1;32m")
    SET(BoldYellow "${Esc}[1;33m")
    SET(BoldBlue "${Esc}[1;34m")
    SET(BoldMagenta "${Esc}[1;35m")
    SET(BoldCyan "${Esc}[1;36m")
    SET(BoldWhite "${Esc}[1;37m")
ENDIF ()
######## PROJECT LIBRARY (NATIVE)
OPTION(BUILD_NATIVE "Build Native" ON) # ON/OFF
IF (BUILD_NATIVE)
    MESSAGE(CHECK_START "${.y}CMaking ${PROJECT_NAME} (NATIVE)${..}")
    IF (NOT CMAKE_BUILD_TYPE)
        SET(CMAKE_BUILD_TYPE "RELEASE")
    ENDIF ()
    MESSAGE(STATUS "Build type: ${.g}${CMAKE_BUILD_TYPE}${..}")
    INCLUDE_DIRECTORIES(src include)
    ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCE_FILES})
    SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
            OUTPUT_NAME ${PROJECT_NAME}
            SUFFIX ".out"
    )
    TARGET_SOURCES(${PROJECT_NAME} PRIVATE src/main.cpp)
    TARGET_COMPILE_FEATURES(${PROJECT_NAME} PRIVATE cxx_std_20)
    ADD_DEFINITIONS("-DNATIVE" "-Wno-ambiguous-reversed-operator")
    INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/include)
    IF (CMAKE_BUILD_TYPE MATCHES "DEBUG")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -O1 -fno-omit-frame-pointer -g")
    ENDIF ()
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
ENDIF ()
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
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC tsl::ordered_map)
INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/ordered_map-src/include/tsl/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/tsl)
INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/ordered_map-src/include/tsl/ DESTINATION ${CMAKE_SOURCE_DIR}/include/tsl)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE tsl::ordered_map)
MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
####################################
####################################
### MQTT: MQTT NETWORK PROTOCOL IMPLEMENTATION
MESSAGE(CHECK_START "${.y}Making Paho MQTT C++ Library${..}")
FETCHCONTENT_DECLARE(
        paho-mqttpp3
        GIT_REPOSITORY https://github.com/eclipse/paho.mqtt.cpp.git
        GIT_TAG v1.4.0
)
FETCHCONTENT_MAKEAVAILABLE(paho-mqttpp3)
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC paho-mqttpp3)
INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/paho-mqttpp3-src/include/mqtt DESTINATION ${CMAKE_INSTALL_PREFIX}/include/mqtt)
INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/paho-mqttpp3-src/include/mqtt DESTINATION ${CMAKE_SOURCE_DIR}/include/mqtt)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE paho-mqttpp3)
ADD_COMPILE_OPTIONS("-Wno-dev")
MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
####################################
MESSAGE(CHECK_START "${.y}Making Paho MQTT C Library${..}")
FETCHCONTENT_DECLARE(
        paho-mqtt3c
        GIT_REPOSITORY https://github.com/eclipse/paho.mqtt.c.git
        GIT_TAG v1.3.13
)
FETCHCONTENT_MAKEAVAILABLE(paho-mqtt3c)
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC paho-mqtt3c)
INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/paho-mqtt3c-src/include/mqtt DESTINATION ${CMAKE_INSTALL_PREFIX}/include/paho-mqtt3c)
INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/paho-mqtt3c-src/include/mqtt DESTINATION ${CMAKE_SOURCE_DIR}/include/paho-mqtt3c)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE paho-mqtt3c)
MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
####################################

####################################
############# TESTING ##############
####################################
ADD_SUBDIRECTORY(test)

####################################
########## DOCUMENTATION ###########
####################################
ADD_SUBDIRECTORY(docs)