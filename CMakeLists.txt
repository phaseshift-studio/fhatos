INCLUDE(CMakePrintHelpers)
INCLUDE(FetchContent)
INCLUDE(ExternalProject)
CMAKE_MINIMUM_REQUIRED(VERSION 3.30.0)  # CMake version check
PROJECT(fhatos)
##################################
## DEFINE MM-ADT ANSI COLOR DSL ##
##################################
IF(NOT WIN32)
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
ENDIF()
####################################################################################################################
############################################ ANSI ART HEADER #######################################################
####################################################################################################################
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
####################################################################################################################
####################################################################################################################
####################################################################################################################

###################

SET(no-dev) # remove mqtt-c warning for developers of mqtt-c

#SET(CMAKE_C_COMPILER /usr/bin/gcc)
#SET(CMAKE_CXX_COMPILER /usr/bin/g++)
#SET(CMAKE_SYSTEM_NAME Linux)
#SET(CMAKE_C_COMPILER   i586-mingw32msvc-gcc)
#SET(CMAKE_CXX_COMPILER i586-mingw32msvc-g++)

####################################################################
####################### DEFAULT SETTINGS ###########################
MESSAGE(CHECK_START "${.y}Setting Default Values${..}")
####################################################################
IF(NOT EXISTS CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE DEBUG)
ENDIF()
#########################################
IF(NOT EXISTS CMAKE_BINARY_DIR)
    SET(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
ENDIF()
#########################################
IF(NOT EXISTS CMAKE_BUILD_PARALLEL_LEVEL)
    SET(CMAKE_BUILD_PARALLEL_LEVEL 8)
ENDIF()
#########################################
IF(NOT EXISTS FOS_SHOW_GIT_PROGRESS)
    SET(FOS_SHOW_GIT_PROGRESS ON)
ENDIF()
#########################################
IF(WIN32)
    SET(PLATFORM "WIN32")
    ADD_DEFINITIONS("-DWIN32")
ELSEIF(APPLE)
    SET(PLATFORM "APPLE")
    ADD_DEFINITIONS("-DAPPLE")
ELSEIF(LINUX)
    SET(PLATFORM "LINUX")
    ADD_DEFINITIONS("-DLINUX")
ENDIF()
#########################################
IF(CMAKE_BUILD_TYPE MATCHES "DEBUG" AND SANITIZER)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -O1 -fno-omit-frame-pointer -g")
ENDIF()
##########################################
OPTION(FOS_SKIP_CMAKE "abort the cnake build process" OFF)
OPTION(CHECK_INTERNET "check internet" ON)
OPTION(BUILD_TESTS "build fhatos tests" ON)
OPTION(BUILD_DOCS "build fhatos website/docs" OFF)
OPTION(SANITIZER "use address sanitizer" OFF)
OPTION(USE_CCACHE "use ccache" ON)
OPTION(USE_CONAN "conan package manager" OFF)
OPTION(USE_PAHO_MQTT_HACK "fix paho mqtt build process" OFF)
OPTION(FOS_SHOW_GIT_PROGRESS "verbose git output on fetch content" ON)
####################################################################
####################################################################
MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
####################################################################
MESSAGE(NOTICE "\n\t${.y}Co${.r}M${.b}m${.c}on ${.r}b${.m}U${.c}il${.b}D ${.r}Pa${.m}rA${.y}mE${.g}Te${.r}rS${..}
\t  ${.g}PROJECT_NAME${..}       : ${PROJECT_NAME}
\t  ${.g}CMAKE_BUILD_TYPE${..}   : ${CMAKE_BUILD_TYPE}
\t  ${.g}CMAKE_SOURCE_DIR${..}   : ${CMAKE_SOURCE_DIR}
\t  ${.g}CMAKE_BINARY_DIR${..}   : ${CMAKE_BINARY_DIR}
\t  ${.g}FOS_SKIP_CMAKE${..}     : ${FOS_SKIP_CMAKE}
\t  ${.g}BUILD_TESTS${..}        : ${BUILD_TESTS}
\t  ${.g}BUILD_DOCS${..}         : ${BUILD_DOCS}
\t  ${.g}USE_CONAN${..}          : ${USE_CONAN}
\t  ${.g}SANITIZER${..}          : ${SANITIZER}
\t  ${.g}USE_CCACHE${..}         : ${USE_CCACHE}
\t  ${.g}USE_PAHO_MQTT_HACK${..} : ${USE_PAHO_MQTT_HACK}
\t  \t${.m}--------------${..}
\t  ${.y}PLATFORM${..}                   : ${PLATFORM}
\t  ${.y}CMAKE_C_COMPILER${..}           : ${CMAKE_C_COMPILER} (${.c}${CMAKE_C_COMPILER_ID}${..})
\t  ${.y}CMAKE_CXX_COMPILER${..}         : ${CMAKE_CXX_COMPILER} (${.c}${CMAKE_CXX_COMPILER_ID}${..})
\t  ${.y}CMAKE_C_COMPILER_VERSION${..}   : ${CMAKE_C_COMPILER_VERSION}
\t  ${.y}CMAKE_CXX_COMPILER_VERSION${..} : ${CMAKE_CXX_COMPILER_VERSION}
\t  ${.y}CMAKE_BUILD_PARALLEL_LEVEL${..} : ${CMAKE_BUILD_PARALLEL_LEVEL}
")
# CMAKE_PRINT_VARIABLES(CMAKE_INSTALL_PREFIX CMAKE_PREFIX_PATH CMAKE_MODULE_PATH CMAKE_CXX_COMPILER_LAUNCHER CMAKE_BUILD_PARALLEL_LEVEL no-dev)
##################
IF(FOS_SKIP_CMAKE)
    MESSAGE(NOTICE "\t\t ${.R}FOS_SKIP_CMAKE${..} is ${.Y}${FOS_SKIP_CMAKE}${..}")
    MESSAGE(NOTICE "\t\t\t${.m}s${.r}H${.c}u${.b}Tt${.y}i${.m}N${.c}g D${.b}o${.r}W${.m}n${..}\n")
    RETURN()
ENDIF()
##################
IF(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    MESSAGE(FATAL_ERROR "In-source builds not allowed. Create a fhatos/build directory and run CMake from there (cmake ..).")
ENDIF()
##################
IF(SANITIZER)
    MESSAGE(NOTICE "  ${.m}.${.r}-${.y}= ${.g}Using Address Sanitizer ${.c}(${.y}-D${.g}SANITIZER${.y}=${.g}ON${.c}) =${.r}-${.m}.${..}")
ELSE()
    MESSAGE(NOTICE "  ${.m}.${.g}-${.y}= ${.r}NOT Using Address Sanitizer ${.y}(${.y}-D${.g}SANITIZER${.y}=${.g}OFF${.c}) =${.g}-${.m}.${..}")
ENDIF()

##################################
## TEST FOR INTERNET CONNECTION ##
##################################
IF(CHECK_INTERNET)
    MESSAGE(CHECK_START "${.y}Testing Internet connection${..}")
    IF(MSVC)
        EXECUTE_PROCESS(
                COMMAND ping www.google.com -n 2
                ERROR_QUIET
                RESULT_VARIABLE NO_CONNECTION
        )
    ELSE()
        EXECUTE_PROCESS(
                COMMAND ping www.google.com -c 2
                ERROR_QUIET
                RESULT_VARIABLE NO_CONNECTION
        )
    ENDIF()
    IF(NOT NO_CONNECTION EQUAL 0)
        SET(FETCHCONTENT_FULLY_DISCONNECTED ON)
        MESSAGE(WARNING "${.r}\\__Fetch offline mode${..}: requires already populated _deps")
    ELSE()
        SET(FETCHCONTENT_FULLY_DISCONNECTED OFF)
        MESSAGE(NOTICE "${.g}\\__Fetch online mode${..}: automatically populates _deps")
    ENDIF()
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
ELSE()
    SET(FETCHCONTENT_FULLY_DISCONNECTED OFF)
ENDIF()
##################################
IF(USE_CONAN AND EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    INCLUDE(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
ELSE()
    MESSAGE(WARNING "\n${.w}The file ${.y}conanbuildinfo.cmake${..} ${.w}doesn't exist${..}.\n\t\tRun ${.b}conan install${..}.")
ENDIF()
################################
SET(ENV{no-dev})
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_FLAGS "-Wall -Wextra")
SET(CMAKE_CXX_FLAGS_DEBUG "-g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
######## PROJECT LIBRARY (NATIVE)
OPTION(BUILD_NATIVE "Build Native" ON) # ON/OFF
IF(BUILD_NATIVE)
    IF(APPLE)
        SET(CMAKE_MACOSX_RPATH 1)
    ENDIF()
    MESSAGE("\n${.g}=====> ${.r}P${.y}R${.m}O${.y}C${.b}E${.m}S${.g}S${.c}I${.y}N${.r}G ${.m}N${.c}A${.g}T${.r}I${.y}V${.b}E${..}\n")
    MESSAGE(CHECK_START "${.y}CMaking ${PROJECT_NAME} (${.r}${PLATFORM}${..}${.y})${..} (${.y}-D${.g}NATIVE${..})")
    MESSAGE(STATUS "${.y}Build type${..}: ${.g}${CMAKE_BUILD_TYPE}${..}")
    IF(USE_CCACHE)
        SET(CMAKE_CXX_COMPILER_LAUNCHER ccache)
    ENDIF()
    INCLUDE_DIRECTORIES(src)
    file(GLOB_RECURSE SOURCE_FILES RELATIVE ${CMAKE_SOURCE_DIR} "src/*.cpp" "src/*.hpp")
    ADD_EXECUTABLE(${PROJECT_NAME} ${SOURCE_FILES} src/main.cpp)
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
    SET(HEADER_FILES "<any>;<atomic>;<deque>;<functional>;<map>;<memory>;<optional>;<queue>;<map>;<set>;<string>;<tsl/ordered_map.h>")
    MESSAGE(STATUS "${.y}Processing precompiled headers: ${.g}${HEADER_FILES}${..}")
    TARGET_PRECOMPILE_HEADERS(${PROJECT_NAME} PUBLIC ${HEADER_FILES})
    ###############################
    IF(APPLE)
        ADD_COMPILE_OPTIONS("-Wno-ambiguous-reversed-operator") # a=b potentially not equivalent to b=a (operator overloading of =)
    ENDIF()
    ADD_DEFINITIONS("-DNATIVE")
    FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include)
    INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/include)
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
ENDIF()
####################################
######## EXTERNAL LIBRARIES ########
####################################
IF(USE_CONAN)
    TARGET_LINK_LIBRARIES($executable_name ${CONAN_LIBS})
ELSE()
    ####################################
    ####################################
    ### FTXUI: ANSI C++ TERMINAL UI
    IF(DO_NOT)
        MESSAGE(CHECK_START "${.y}Making FXTUI Library${..}")
        SET(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
        FETCHCONTENT_DECLARE(
                ftxui
                GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
                GIT_TAG v5.0.0
                GIT_PROGRESS ${FOS_SHOW_GIT_PROGRESS}
        )
        FETCHCONTENT_MAKEAVAILABLE(ftxui)
        TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ftxui)
        INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/ftxui-src/include/ftxui/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/ftxui)
        INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/ftxui-src/include/ftxui/ DESTINATION ${CMAKE_SOURCE_DIR}/include/ftxui)
        TARGET_LINK_LIBRARIES(fhatos
                PRIVATE ftxui::screen
                PRIVATE ftxui::dom
                PRIVATE ftxui::component # Not needed for this example.
        )
        MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
    ENDIF()
    ####################################
    ####################################
    ### ORDERED_MAP: INSERT ORDER MAP IMPLEMENTATION
    MESSAGE(CHECK_START "${.y}Making Ordered Map Library${..}")
    FETCHCONTENT_DECLARE(
            ordered_map
            GIT_REPOSITORY https://github.com/Tessil/ordered-map.git
            GIT_TAG v1.1.0
            GIT_PROGRESS ${FOS_SHOW_GIT_PROGRESS}
    )
    FETCHCONTENT_MAKEAVAILABLE(ordered_map)
    INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/ordered_map-src/include/tsl/ DESTINATION ${CMAKE_BINARY_DIR}/include/tsl)
    #FILE(COPY ${CMAKE_BINARY_DIR}/_deps/ordered_map-src/include/tsl/ DESTINATION ${CMAKE_BINARY_DIR}/include/tsl)
    TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/include/tsl)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE ordered_map)
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
            GIT_PROGRESS ${FOS_SHOW_GIT_PROGRESS}
    )
    FETCHCONTENT_MAKEAVAILABLE(paho-mqtt3c)
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
    #################################### C++
    MESSAGE(CHECK_START "${.y}Making Paho MQTT C++ Library${..}")
    FETCHCONTENT_DECLARE(
            paho-mqttpp3
            GIT_REPOSITORY https://github.com/eclipse/paho.mqtt.cpp.git
            GIT_TAG v1.4.1
            GIT_PROGRESS ${FOS_SHOW_GIT_PROGRESS}
    )
    ##################################################################
    ##### HACK TO FIX LIBRARY NAMING ERRORS IN PAHO MQTT RELEASE #####
    ##################################################################
    IF(USE_PAHO_MQTT_HACK)
        MESSAGE(CHECK_START "${.r}Performing Paho-MQTT Build Hack ${.g}[${.y}PART 1${.g}]${..}")
        SET(PAHO_MQTTPP3_DIR ${CMAKE_BINARY_DIR}/_deps/paho-mqttpp3-src/)
        FILE(REMOVE ${PAHO_MQTTPP3_DIR}/CMakeLists.txt)
        FILE(COPY ${CMAKE_SOURCE_DIR}/CMakeLists-paho-mqtt-fix.txt DESTINATION ${PAHO_MQTTPP3_DIR})
        FILE(RENAME ${PAHO_MQTTPP3_DIR}/CMakeLists-paho-mqtt-fix.txt ${PAHO_MQTTPP3_DIR}/CMakeLists.txt)
        FILE(REMOVE ${PAHO_MQTTPP3_DIR}/src/CMakeLists.txt)
        FILE(COPY ${CMAKE_SOURCE_DIR}/CMakeLists-paho-mqtt-src-fix.txt DESTINATION ${PAHO_MQTTPP3_DIR}/src)
        FILE(RENAME ${PAHO_MQTTPP3_DIR}/src/CMakeLists-paho-mqtt-src-fix.txt ${PAHO_MQTTPP3_DIR}/src/CMakeLists.txt)
        MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
    ENDIF()
    ##################################################################
    ##################################################################
    ##################################################################
    FETCHCONTENT_MAKEAVAILABLE(paho-mqttpp3)
    INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/paho-mqtt3c-build/src DESTINATION ${CMAKE_INSTALL_PREFIX}/include/mqtt-c/src)
    INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/paho-mqttpp3-src/include DESTINATION ${CMAKE_INSTALL_PREFIX}/include/mqtt-cpp)
    IF(USE_PAHO_MQTT_HACK)
        MESSAGE(CHECK_START "${.r}Performing Paho-MQTT Build Hack ${.g}[${.y}PART DEUX${.g}]${..}")
        FILE(COPY ${CMAKE_BINARY_DIR}/_deps/paho-mqttpp3-build/src/ DESTINATION ${CMAKE_BINARY_DIR}/include/mqtt-cpp)
        FILE(COPY ${CMAKE_BINARY_DIR}/_deps/paho-mqtt3c-build/src/ DESTINATION ${CMAKE_BINARY_DIR}/include/mqtt-c)
        FILE(COPY ${CMAKE_BINARY_DIR}/_deps/paho-mqtt3c-src/src/ DESTINATION ${CMAKE_BINARY_DIR}/include/mqtt-c/src)
        SET(PAHO_MQTT_C_LIB ${CMAKE_BINARY_DIR}/include/mqtt-c/libpaho-mqtt3c.so.1.3.13)
        INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/include/mqtt-c)
        INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/include/mqtt-c/src/)
        INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/include/mqtt-cpp)
        TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC
                ${CMAKE_BINARY_DIR}/include/mqtt-c/libpaho-mqtt3c.so.1.3.13
                ${CMAKE_BINARY_DIR}/include/mqtt-c/libpaho-mqtt3a.so.1.3.13
                ${CMAKE_BINARY_DIR}/include/mqtt-c/libpaho-mqtt3a.so.1
                paho-mqttpp3)
        MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
    ELSE()
        TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC paho-mqtt3c paho-mqttpp3)
    ENDIF()
    TARGET_LINK_DIRECTORIES(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/include/mqtt-c/src)
    SET(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/include/mqtt-c/src)
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
    ####################################
    ####################################
    ### MPC: MICRO PARSER COMBINATORS
    IF(DO_NOT)
        MESSAGE(CHECK_START "${.y}Making Micro Parser Combinators Library${..}")
        FETCHCONTENT_DECLARE(
                mpc
                GIT_REPOSITORY https://github.com/orangeduck/mpc.git
                GIT_TAG 0.9.0
                GIT_PROGRESS ${FOS_SHOW_GIT_PROGRESS}
        )
        FETCHCONTENT_MAKEAVAILABLE(mpc)
        TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC mpc)
        INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/mpc-src DESTINATION ${CMAKE_INSTALL_PREFIX}/include/mpc)
        INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/mpc-src DESTINATION ${CMAKE_SOURCE_DIR}/include/mpc)
        TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/_deps/mpc-src)
        MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
    ENDIF()
ENDIF()
####################################
############# TESTING ##############
####################################
ADD_SUBDIRECTORY(test)

####################################
########## DOCUMENTATION ###########
####################################
ADD_SUBDIRECTORY(docs)