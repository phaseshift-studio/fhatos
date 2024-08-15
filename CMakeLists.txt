INCLUDE(CMakePrintHelpers)
INCLUDE(FetchContent)
CMAKE_MINIMUM_REQUIRED(VERSION 3.30.0)  # CMake version check
PROJECT(fhatos)
SET(no-dev) # remove mqtt-c warning for developers of mqtt-c
SET(ENV{no-dev})
OPTION(USE_CCACHE "Use ccache" ON) # ON/OFF
IF (USE_CCACHE)
    SET(CMAKE_CXX_COMPILER_LAUNCHER ccache)
ENDIF ()
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
IF (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    MESSAGE(FATAL_ERROR "In-source builds not allowed. Create a fhatos/build directory and run CMake from there (cmake ..).")
ENDIF ()
IF (NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE "DEBUG")
ENDIF ()
SET(CMAKE_CXX_FLAGS "-Wall -Wextra")
SET(CMAKE_CXX_FLAGS_DEBUG "-g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
##################################
## DEFINE MM-ADT ANSI COLOR DSL ##
##################################
IF (NOT WIN32)
    STRING(ASCII 27 Esc)
    SET(.. "${Esc}[m")
    SET(.r "${Esc}[31m") # red
    SET(.g "${Esc}[32m") # green
    SET(.y "${Esc}[33m") # yellow
    SET(.b "${Esc}[34m") # blue
    SET(.m "${Esc}[35m") # magenta
    SET(.c "${Esc}[36m") # cyan
    SET(.w "${Esc}[37m") # white
    SET(.R "${Esc}[1;31m") # bold versions of colors above
    SET(.G "${Esc}[1;32m")
    SET(.Y "${Esc}[1;33m")
    SET(.B "${Esc}[1;34m")
    SET(.M "${Esc}[1;35m")
    SET(.C "${Esc}[1;36m")
    SET(.W "${Esc}[1;37m")
ENDIF ()
MESSAGE("${.r}            PhaseShift Studio Presents \n"
        "${.m} <`--'>____${.g}  ______ __  __  ______  ______  ${.b}______  ______${..} \n"
        "${.m} /. .  `'  \\${.g}/\\  ___/\\ \\_\\ \\/\\  __ \\/\\__  _\\${.b}/\\  __ \\/\\  "
        "___\\${..} \n"
        "${.m}(`')  ,     ${.m}@${.g} \\  __\\ \\  __ \\ \\  __ \\/_/\\ \\/${.b}\\ \\ \\_\\ \\ "
        "\\___  \\${..} \n"
        "${.m} `-._,     /${.g} \\ \\_\\  \\ \\_\\ \\_\\ \\_\\ \\_\\ \\ \\_\\ ${.b}\\ "
        "\\_____\\/\\_____\\ \n"
        "${.m}    )-)_/-(>${.g}  \\/_/   \\/_/\\/_/\\/_/\\/_/  \\/_/  "
        "${.b}\\/_____/\\/_____/ \n"
        "${.r}                                   A Dogturd Stynx Production${..} \n")
IF (WIN32)
    MESSAGE(NOTICE "${.r}https://cmake.org/cmake/help/latest/variable/WIN32.html?highlight=win32${..}")
ELSEIF (APPLE)
    MESSAGE(NOTICE "${.y}https://cmake.org/cmake/help/latest/variable/APPLE.html?highlight=apple${..}")
ELSEIF (UNIX)
    MESSAGE(NOTICE "${.g}https://cmake.org/cmake/help/latest/variable/UNIX.html?highlight=unix${..}")
ENDIF ()
## PARALLELISM LEVEL (HARDCODED TO 8)
SET(CMAKE_BUILD_PARALLEL_LEVEL 8)
CMAKE_PRINT_VARIABLES(PROJECT_NAME CMAKE_SOURCE_DIR CMAKE_BINARY_DIR CMAKE_INSTALL_PREFIX CMAKE_CXX_COMPILER_ID CMAKE_CXX_COMPILER_VERSION CMAKE_CXX_COMPILER_LAUNCHER CMAKE_BUILD_PARALLEL_LEVEL no-dev)
MESSAGE(STATUS "${.g}Parallel build level${..}: ${.y}${CMAKE_BUILD_PARALLEL_LEVEL}${..}")
##################################
## TEST FOR INTERNET CONNECTION ##
##################################
OPTION(CHECK_INTERNET "Check Internet" ON) # ON/OFF
IF (CHECK_INTERNET)
    MESSAGE(CHECK_START "${.y}Testing Internet connection${..}")
    IF (MSVC)
        EXECUTE_PROCESS(
                COMMAND ping www.google.com -n 2
                ERROR_QUIET
                RESULT_VARIABLE NO_CONNECTION
        )
    ELSE ()
        EXECUTE_PROCESS(
                COMMAND ping www.google.com -c 2
                ERROR_QUIET
                RESULT_VARIABLE NO_CONNECTION
        )
    ENDIF ()
    IF (NOT NO_CONNECTION EQUAL 0)
        SET(FETCHCONTENT_FULLY_DISCONNECTED ON)
        MESSAGE(WARNING "${.r}\\__Fetch offline mode${..}: requires already populated _deps")
    ELSE ()
        SET(FETCHCONTENT_FULLY_DISCONNECTED OFF)
        MESSAGE(NOTICE "${.g}\\__Fetch online mode${..}: automatically populates _deps")
    ENDIF ()
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
ELSE ()
    SET(FETCHCONTENT_FULLY_DISCONNECTED ON)
ENDIF ()
##################################

######## PROJECT LIBRARY (NATIVE)
OPTION(BUILD_NATIVE "Build Native" ON) # ON/OFF
IF (BUILD_NATIVE)
    SET(CMAKE_MACOSX_RPATH 1)
    MESSAGE("\n${.g}=====> ${.r}P${.y}R${.m}O${.y}C${.b}E${.m}S${.g}S${.c}I${.y}N${.r}G ${.m}N${.c}A${.g}T${.r}I${.y}V${.b}E${..}\n")
    MESSAGE(CHECK_START "${.y}CMaking ${PROJECT_NAME} (NATIVE)${..}")
    MESSAGE(STATUS "${.y}Build type${..}: ${.g}${CMAKE_BUILD_TYPE}${..}")
    INCLUDE_DIRECTORIES(src)
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
    ##### PRECOMPILED HEADERS #####
    SET(ROOT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
    SET(HEADER_FILES "<any>;<atomic>;<deque>;<functional>;<map>;<memory>;<optional>;<queue>;<set>;<string>;<tsl/ordered_map.h>;<mqtt/async_client.h>;<libwebsockets.h>")
    MESSAGE(STATUS "${.y}Processing precompiled headers: ${.g}${HEADER_FILES}${..}")
    TARGET_PRECOMPILE_HEADERS(${PROJECT_NAME} PUBLIC ${HEADER_FILES})
    ###############################
    IF (APPLE)
        ADD_COMPILE_OPTIONS("-Wno-ambiguous-reversed-operator") # a=b potentially not equivalent to b=a (operator overloading of =)
    ENDIF ()
    ADD_DEFINITIONS("-DNATIVE")
    FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include)
    INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/include)
    IF (CMAKE_BUILD_TYPE MATCHES "DEBUG")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -O1 -fno-omit-frame-pointer -g")
    ENDIF ()
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
ENDIF ()
####################################
######## EXTERNAL LIBRARIES ########
####################################

####################################
####################################
### LIBWEBSOCKETS: UNIVERSAL WEB PROTOCOL API
OPTION(USE_LIBWEBSOCKETS "Use libwebsockets" OFF) # ON/OFF
IF (USE_LIBWEBSOCKETS)
    MESSAGE(CHECK_START "${.y}CMaking libwebsockets library${..}")
    FIND_PACKAGE(PkgConfig)
    PKG_CHECK_MODULES(LIB_WEBSOCKETS REQUIRED libwebsockets)
    GET_FILENAME_COMPONENT(
            LIB_WEBSOCKETS_INSTALL_DIR
            ${LIB_WEBSOCKETS_LIBRARY_DIRS}
            DIRECTORY
    )
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC
            ${LIB_WEBSOCKETS_LIBRARIES})
    #SET_TARGET_PROPERTIES(
    #        ${PROJECT_NAME}
    #        PROPERTIES
    #        INCLUDE_DIRECTORIES
    #        ${LIB_WEBSOCKETS_INCLUDE_DIRS}
    #        LINK_FLAGS
    #        "-L${LIB_WEBSOCKETS_LIBRARY_DIRS}"
    #        COMPILE_DEFINITIONS
    #        INSTALL_DATADIR="${LIB_WEBSOCKETS_INSTALL_DIR}/share"
    #)
    CMAKE_PRINT_VARIABLES(LIB_WEBSOCKETS_LIBRARIES,LIB_WEBSOCKETS_INCLUDE_DIRS)
    MESSAGE(STATUS "${.y}libwebsockets libs${..}: ${.g}${LIB_WEBSOCKETS_INCLUDE_DIRS}${..}")
    #ADD_DEPENDENCIES(${PROJECT_NAME}
    #        websockets_shared)
    FILE(COPY ${LIB_WEBSOCKETS_INCLUDE_DIRS} DESTINATION ${CMAKE_BINARY_DIR}/_deps/libwebsockets-src)
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
ENDIF ()
####################################
####################################
### YAML: YAML parser_ IMPLEMENTATION
OPTION(USE_YAML "YAML build" OFF) # ON/OFF
IF (USE_YAML)
    FETCHCONTENT_DECLARE(
            yaml-cpp
            GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
            GIT_TAG 0.8.0
    )
    FETCHCONTENT_GETPROPERTIES(yaml-cpp)
    IF (NOT yaml-cpp_POPULATED)
        MESSAGE(STATUS "Fetching yaml-cpp...")
        FETCHCONTENT_MAKEAVAILABLE(yaml-cpp)
        #ADD_SUBDIRECTORY(${yaml-cpp_SOURCE_DIR} ${yaml-cpp_BINARY_DIR})
    ENDIF ()
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC yaml-cpp::yaml-cpp)
ENDIF ()
####################################
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
#################################### C
MESSAGE(CHECK_START "${.y}Making Paho MQTT C Library${..}")
FETCHCONTENT_DECLARE(
        paho-mqtt3c
        GIT_REPOSITORY https://github.com/eclipse/paho.mqtt.c.git
        GIT_TAG v1.3.13
)
FETCHCONTENT_MAKEAVAILABLE(paho-mqtt3c)
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC paho-mqtt3c)
INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/paho-mqtt3c-src/include/mqtt DESTINATION ${CMAKE_INSTALL_PREFIX}/include/mqtt)
INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/paho-mqtt3c-src/include/mqtt DESTINATION ${CMAKE_SOURCE_DIR}/include/mqtt)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE paho-mqtt3c)
MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
#################################### C++
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