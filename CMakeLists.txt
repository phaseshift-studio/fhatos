CMAKE_MINIMUM_REQUIRED(VERSION 3.25.0)  # CMake version check
###### CMAKE INCLUDES
INCLUDE(CMakePrintHelpers)
INCLUDE(FetchContent)
INCLUDE(ExternalProject)
###### CMAKE CPACK INCLUDE
SET(CPACK_PACKAGING_INSTALL_PREFIX "/opt/${CPACK_PACKAGE_NAME}/${CMAKE_PROJECT_VERSION}")
SET(CPACK_SET_DESTDIR ${CMAKE_BINARY_DIR})
INCLUDE(CPack)

######## The fHatTy #########
PROJECT(fhatos) #############
#############################

##############################################
############ RELEASE DEFINITIONS #############
##############################################
SET(FOS_NAME FhatOS)
SET(FOS_VERSION 0.1-alpha)
ADD_DEFINITIONS("-DFOS_NAME=${FOS_NAME}")
ADD_DEFINITIONS("-DFOS_VERSION=${FOS_VERSION}")
##############################################
##############################################

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
    SET(.FHATOS "${.m}F${.y}h${.b}a${.r}t${.y}O${.c}S")
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

####################################################################
####################### DEFAULT SETTINGS ###########################
MESSAGE(CHECK_START "${.y}setting default values${..}")
####################################################################
SET(no-dev) # remove mqtt-c warning for developers of mqtt-c
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
SET(FOS_MACHINE_MODEL ${CMAKE_HOST_SYSTEM_PROCESSOR})
IF(WIN32)
    SET(PLATFORM WIN32)
    ADD_DEFINITIONS(-DWIN32)
ELSEIF(APPLE)
    SET(PLATFORM APPLE)
    ADD_DEFINITIONS(-DAPPLE)
ELSEIF(LINUX)
    SET(PLATFORM LINUX)
    ADD_DEFINITIONS(-DLINUX)
    IF(${CMAKE_SYSTEM_PROCESSOR} MATCHES "armv[67]l" OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
        OPTION(RASPBERRYPI "build to raspberrypi" ON)
        ADD_DEFINITIONS(-DRASPBERRYPI)
        FILE(READ "/sys/firmware/devicetree/base/model" FOS_MACHINE_MODEL)
        STRING(TOLOWER ${FOS_MACHINE_MODEL} FOS_MACHINE_MODEL)
        MESSAGE(NOTICE
                "\tbuilding for ${.r}R${.y}a${.b}s${.r}p${.m}b${.c}r${.y}r${.b}y ${.r}P${.y}i${..}
\t  ${.y}FOS_MACHINE_MODEL${..} : ${FOS_MACHINE_MODEL}")
        STRING(FIND ${FOS_MACHINE_MODEL} "nano" X)
        IF(${X} GREATER -1)
            OPTION(NANOPI "nanopi architecture" ON)
            ADD_DEFINITIONS(-DNANOPI)
        ENDIF()
        STRING(FIND ${FOS_MACHINE_MODEL} "orange" X)
        IF(${X} GREATER -1)
            OPTION(ORANGEPI "orangepi architecture" ON)
            ADD_DEFINITIONS(-DORANGEPI)
        ENDIF()
    ENDIF()
ENDIF()
#########################################
IF(CMAKE_BUILD_TYPE MATCHES "DEBUG" AND SANITIZER)
    SET(CMAKE_CXX_FLAGS "-fsanitize=address -O1 -fno-omit-frame-pointer -g ${CMAKE_CXX_FLAGS}")
ENDIF()
FIND_PROGRAM(CCACHE_FOUND ccache)
IF(CCACHE_FOUND)
    MESSAGE(NOTICE "${.y}ccache${..} found at ${CCACHE_FOUND}")
    OPTION(USE_CCACHE "use ccache" ON)
ELSE()
    MESSAGE(WARNING "${.y}ccache${..} not found")
    SET(USE_CCACHE OFF)
ENDIF()
################################
SET(ENV{no-dev})
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_C_COMPILER_VERSION 14)
SET(CMAKE_CXX_COMPILER_VERSION 14)
SET(CMAKE_CXX_FLAGS "-Wall -Wextra ${CMAKE_CXX_FLAGS}")
SET(CMAKE_CXX_FLAGS_DEBUG "-g ${CMAKE_CXX_FLAGS_DEBUG}")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 ${CMAKE_CXX_FLAGS_RELEASE}")
####################################################################
####################################################################
####################################################################
OPTION(FOS_SKIP_CMAKE "abort the cmake build process" OFF)
OPTION(FOS_LOGGING "fhatos default logging" INFO)
OPTION(CHECK_INTERNET "check internet" ON)
OPTION(BUILD_TESTS "build fhatos tests" OFF)
OPTION(BUILD_DOCS "build fhatos website/docs" OFF)
OPTION(SANITIZER "use address sanitizer" OFF)
OPTION(USE_CCACHE "use ccache" ON)
OPTION(USE_PAHO_MQTT_HACK "fix paho mqtt build process" ON)
OPTION(FOS_SHOW_GIT_PROGRESS "verbose git output on fetch content" ON)
MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
####################################################################
####################### CPACK PACKAGING ############################
####################################################################
MESSAGE(CHECK_START "${.y}configuring packaging with cpack${..}")
SET(CPACK_PACKAGE_NAME ${PROJECT_NAME})
SET(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
SET(CPACK_PACKAGING_INSTALL_PREFIX "/opt/${CPACK_PACKAGE_NAME}/${CMAKE_PROJECT_VERSION}")
SET(CPACK_SET_DESTDIR ${CMAKE_BINARY_DIR})
SET(CPACK_GENERATOR DEB)
SET(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "FhatOS Distributed Operating System")
SET(CPACK_PACKAGE_VENDOR "fhatos.org")
SET(CPACK_PACKAGE_CONTACT "dogturd.stynx@fhatos.org")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Dr. Stynx")
SET(CPACK_VERBATIM_VARIABLES ON)
SET(CPACK_PACKAGE_INSTALL_DIRECTORY ${CMAKE_BINARY_DIR})
SET(CPACK_OUTPUT_FILE_PREFIX ${CMAKE_BINARY_DIR})
SET(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION})
SET(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION})
SET(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION})
SET(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
SET(CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/README.md")
SET(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
####################################################################
SET(FOS_MACHINE_SUBOS ${CMAKE_SYSTEM})
SET(FOS_MACHINE_ARCH ${CMAKE_SYSTEM_PROCESSOR})
MESSAGE(NOTICE "
\t  \t${.m}-----${.y}Co${.r}M${.b}m${.c}on ${.r}b${.m}U${.c}il${.b}D ${.r}Pr${.m}op${.y}eR${.g}Ti${.r}Es${.m}-----${..}
\t  ${.g}PROJECT_NAME${..}       : ${PROJECT_NAME}
\t  ${.g}CMAKE_BUILD_TYPE${..}   : ${CMAKE_BUILD_TYPE}
\t  ${.g}CMAKE_SOURCE_DIR${..}   : ${CMAKE_SOURCE_DIR}
\t  ${.g}CMAKE_BINARY_DIR${..}   : ${CMAKE_BINARY_DIR}
\t  ${.g}FOS_SKIP_CMAKE${..}     : ${FOS_SKIP_CMAKE}
\t  ${.g}FOS_LOGGING${..}        : ${FOS_LOGGING}
\t  ${.g}CHECK_INTERNET${..}     : ${CHECK_INTERNET}
\t  ${.g}BUILD_TESTS${..}        : ${BUILD_TESTS}
\t  ${.g}BUILD_DOCS${..}         : ${BUILD_DOCS}
\t  ${.g}SANITIZER${..}          : ${SANITIZER}
\t  ${.g}USE_CCACHE${..}         : ${USE_CCACHE}
\t  ${.g}USE_PAHO_MQTT_HACK${..} : ${USE_PAHO_MQTT_HACK}

\t  \t${.m}-----${.c}S${.y}T${.r}A${.c}T${.m}I${.b}C ${.r}pR${.m}OP${.y}Er${.g}tI${.r}eS${.m}-----${..}
\t  ${.y}FOS_NAME FOS_VERSION${..}       : ${FOS_NAME} ${FOS_VERSION}
\t  ${.y}FOS_MACHINE_SUBOS${..}          : ${FOS_MACHINE_SUBOS}
\t  ${.y}FOS_MACHINE_ARCH${..}           : ${FOS_MACHINE_ARCH}
\t  ${.y}FOS_MACHINE_MODEL${..}          : ${FOS_MACHINE_MODEL}
\t  ${.y}PLATFORM${..}                   : ${PLATFORM}
\t  ${.y}CMAKE_SYSTEM_NAME${..}          : ${CMAKE_SYSTEM_NAME}
\t  ${.y}CMAKE_C_COMPILER${..}           : ${CMAKE_C_COMPILER} (${.c}${CMAKE_C_COMPILER_ID}${..})
\t  ${.y}CMAKE_CXX_COMPILER${..}         : ${CMAKE_CXX_COMPILER} (${.c}${CMAKE_CXX_COMPILER_ID}${..})
\t  ${.y}CMAKE_C_COMPILER_VERSION${..}   : ${CMAKE_C_COMPILER_VERSION}
\t  ${.y}CMAKE_CXX_COMPILER_VERSION${..} : ${CMAKE_CXX_COMPILER_VERSION}
\t  ${.y}CMAKE_BUILD_PARALLEL_LEVEL${..} : ${CMAKE_BUILD_PARALLEL_LEVEL}
\t  ${.y}CMAKE_CXX_FLAGS${..}            : ${CMAKE_CXX_FLAGS}
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
    MESSAGE(FATAL_ERROR "in-source builds not allowed. create a fhatos/build directory and run CMake from there (cmake ..).")
ENDIF()
##################
IF(SANITIZER)
    MESSAGE(NOTICE "  ${.m}.${.r}-${.y}= ${.g}using address sanitizer ${.c}(${.y}-D${.g}SANITIZER${.y}=${.g}ON${.c}) =${.r}-${.m}.${..}")
ELSE()
    MESSAGE(NOTICE "  ${.m}.${.g}-${.y}= ${.r}NOT using address sanitizer ${.y}(${.y}-D${.g}SANITIZER${.y}=${.g}OFF${.c}) =${.g}-${.m}.${..}")
ENDIF()

##################################
## TEST FOR INTERNET CONNECTION ##
##################################
IF(CHECK_INTERNET)
    MESSAGE(CHECK_START "${.y}testing internet connection${..}")
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
        MESSAGE(WARNING "${.r}\\__fetch offline mode${..}: requires already populated _deps")
    ELSE()
        SET(FETCHCONTENT_FULLY_DISCONNECTED OFF)
        MESSAGE(NOTICE "${.g}\\__fetch online mode${..}: automatically populates _deps")
    ENDIF()
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
ELSE()
    SET(FETCHCONTENT_FULLY_DISCONNECTED OFF)
ENDIF()
#############################################################################################################
#############################################################################################################
#############################################################################################################
######## PROJECT LIBRARY (NATIVE)
OPTION(BUILD_NATIVE "build native" ON) # ON/OFF
IF(BUILD_NATIVE)
    IF(APPLE)
        SET(CMAKE_MACOSX_RPATH 1)
    ENDIF()
    MESSAGE("\n${.g}=====> ${.r}P${.y}R${.m}O${.y}C${.b}E${.m}S${.g}S${.c}I${.y}N${.r}G ${.m}N${.c}A${.g}T${.r}I${.y}V${.b}E${..}\n")
    MESSAGE(CHECK_START "${.y}making ${PROJECT_NAME} (${.r}${PLATFORM}${..}${.y})${..} (${.y}-D${.g}NATIVE${..})")
    MESSAGE(STATUS "${.y}build type${..}: ${.g}${CMAKE_BUILD_TYPE}${..}")
    IF(USE_CCACHE)
        SET(CMAKE_CXX_COMPILER_LAUNCHER ccache)
    ENDIF()
    INCLUDE_DIRECTORIES(src)
    FILE(GLOB_RECURSE SOURCE_FILES RELATIVE ${CMAKE_SOURCE_DIR} "src/*.cpp" "src/*.hpp")
    IF(VERBOSE)
        MESSAGE(NOTICE "${.g}adding source files${..}: ${SOURCE_FILES}")
    ENDIF()
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
    STRING(CONCAT HEADER_FILES
            "<any>;<atomic>;<deque>;<functional>;<vector>;<map>;<memory>;<optional>;"
            "<queue>;<map>;<set>;<string>;<ostream>;<shared_mutex>;<utility>;<variant>")
    MESSAGE(STATUS "${.y}processing precompiled headers: ${.g}${HEADER_FILES}${..}")
    TARGET_PRECOMPILE_HEADERS(${PROJECT_NAME} PUBLIC ${HEADER_FILES})
    ###############################
    IF(APPLE)
        ADD_COMPILE_OPTIONS("-Wno-ambiguous-reversed-operator") # a=b potentially not equivalent to b=a (operator overloading of =)
    ENDIF()
    ADD_DEFINITIONS(
            -DNATIVE
            -DBUILD_DOCS=${BUILD_DOCS}
            -DFOS_LOGGING=${FOS_LOGGING}
            -DFOS_MACHINE_SUBOS=${FOS_MACHINE_SUBOS}
            -DFOS_MACHINE_ARCH=${FOS_MACHINE_ARCH}
            -DFOS_MACHINE_MODEL=${FOS_MACHINE_MODEL})
    FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include)
    INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/include)
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
ENDIF()
##############################################################################################################
##############################################################################################################
##############################################################################################################

####################################
######## EXTERNAL LIBRARIES ########
####################################

####################################
####################################
### ORDERED_MAP: INSERT ORDER MAP IMPLEMENTATION
MESSAGE(CHECK_START "${.y}making ordered map library${..}")
MESSAGE(NOTICE "\t${.r}IMPORTANT${..}: ${.m}ordered-map${..} distributed w/ ${.FHATOS} via ${.b}util/tsl/ordered-map.h${..}.")
IF(DO_NOT)
    FETCHCONTENT_DECLARE(
            ordered_map
            GIT_REPOSITORY https://github.com/Tessil/ordered-map.git
            GIT_TAG v1.1.0
            GIT_PROGRESS ${FOS_SHOW_GIT_PROGRESS})
    FETCHCONTENT_MAKEAVAILABLE(ordered_map)
    INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/ordered_map-src/include/tsl/ DESTINATION ${CMAKE_BINARY_DIR}/include/tsl)
    FILE(COPY ${CMAKE_BINARY_DIR}/_deps/ordered_map-src/include/tsl/ DESTINATION ${CMAKE_BINARY_DIR}/include/tsl)
    TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/include/tsl)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE ordered_map)
ENDIF()
MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")

####################################
####################################
### MQTT: MQTT NETWORK PROTOCOL IMPLEMENTATION
####################################
MESSAGE(CHECK_START "${.y}making paho mqtt C library${..}")
SET(PAHO_ENABLE_TESTING OFF)
SET(PAHO_BUILD_TESTS OFF)
SET(PAHO_BUILD_DOCUMENTATION OFF)
FETCHCONTENT_DECLARE(
        paho-mqtt3c
        GIT_REPOSITORY https://github.com/eclipse/paho.mqtt.c.git
        GIT_TAG v1.3.13
        GIT_PROGRESS ${FOS_SHOW_GIT_PROGRESS})
FETCHCONTENT_MAKEAVAILABLE(paho-mqtt3c)
MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
#################################### C++
MESSAGE(CHECK_START "${.y}making paho mqtt C++ library${..}")
FETCHCONTENT_DECLARE(
        paho-mqttpp3
        GIT_REPOSITORY https://github.com/eclipse/paho.mqtt.cpp.git
        GIT_TAG v1.4.1
        GIT_PROGRESS ${FOS_SHOW_GIT_PROGRESS})
##################################################################
##### HACK TO FIX LIBRARY NAMING ERRORS IN PAHO MQTT RELEASE #####
##################################################################
IF(USE_PAHO_MQTT_HACK)
    MESSAGE(CHECK_START "${.r}performing paho-mqtt build hack ${.g}[${.y}PART 1${.g}]${..}")
    MESSAGE(NOTICE "\t${.y}IMPORTANT${..}: ${.g}paho${..}-${.b}mqtt${..} build hack ${.r}may required${..} to be built ${.r}twice${..}.")
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
    MESSAGE(CHECK_START "${.r}performing paho-mqtt build hack ${.g}[${.y}PART DEUX${.g}]${..}")
    FILE(COPY ${CMAKE_BINARY_DIR}/_deps/paho-mqttpp3-build/src/ DESTINATION ${CMAKE_BINARY_DIR}/include/mqtt-cpp)
    FILE(COPY ${CMAKE_BINARY_DIR}/_deps/paho-mqtt3c-build/src/ DESTINATION ${CMAKE_BINARY_DIR}/include/mqtt-c)
    FILE(COPY ${CMAKE_BINARY_DIR}/_deps/paho-mqtt3c-src/src/ DESTINATION ${CMAKE_BINARY_DIR}/include/mqtt-c/src)
    TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/include/mqtt-c)
    TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/include/mqtt-c/src/)
    TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/include/mqtt-cpp)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC paho-mqtt3a paho-mqttpp3)
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
ELSE()
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC paho-mqtt3c paho-mqttpp3)
ENDIF()
TARGET_LINK_DIRECTORIES(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/include/mqtt-c/src)
SET(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR}/include/mqtt-c/src)
MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
####################################
####################################
### WIRING PI: GPIO ACCESS FOR RASPBERRY PI
IF(RASPBERRYPI)
    MESSAGE(CHECK_START "${.y}making raspberry pi wiring library${..}")
    IF(NANOPI)
        SET(WIRINGXX_GIT_REPO https://github.com/friendlyarm/WiringNP)
        SET(WIRINGXX_TAG master)
        SET(XX NP)
    ELSEIF(ORANGEPI)
        SET(WIRINGXX_GIT_REPO https://github.com/orangepi-xunlong/wiringOP.git)
        SET(WIRINGXX_TAG v0.2)
        SET(XX OP)
    ELSE()
        SET(WIRINGXX_GIT_REPO https://github.com/WiringPi/WiringPi.git)
        SET(WIRINGXX_TAG 3.10)
        SET(XX PI)
    ENDIF()
    MESSAGE(NOTICE "${.y}building ${.g}Wiring${XX} ${.y}at ${.b}${WIRINGXX_GIT_REPO}${..}")
    FETCHCONTENT_DECLARE(
            wiringxx
            GIT_REPOSITORY ${WIRINGXX_GIT_REPO}
            GIT_TAG ${WIRINGXX_TAG}
            GIT_PROGRESS ${FOS_SHOW_GIT_PROGRESS}
    )
    FETCHCONTENT_MAKEAVAILABLE(wiringxx)
    FILE(COPY ${CMAKE_BINARY_DIR}/_deps/wiringxx-src/ DESTINATION ${CMAKE_BINARY_DIR}/include/wiringxx)
    SET(WIRINGXX_LOCATION ${CMAKE_BINARY_DIR}/include/wiringxx)
    EXECUTE_PROCESS(
            WORKING_DIRECTORY ${WIRINGXX_LOCATION}
            COMMAND ./build
            OUTPUT_VARIABLE WIRINGXX_BUILD_RESULT
    )
    MESSAGE(NOTICE ${WIRINGXX_BUILD_RESULT})
    FIND_LIBRARY(WIRINGXX_LIBRARIES NAMES wiringPi)
    TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${WIRINGXX_LOCATION}/wiring${XX})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC ${WIRINGXX_LIBRARIES})
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
ENDIF()

####################################
####################################
### PEGLIB: A C++ PEG PARSER
MESSAGE(CHECK_START "${.y}making cpp-peglib library${..}")
MESSAGE(NOTICE "\t${.r}IMPORTANT${..}: ${.m}cpp-peglib${..} distributed w/ ${.FHATOS} via ${.b}language/util/peglib.h${..} (altered for microprocessor architectures).")
IF(DO_NOT)
    SET(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
    FETCHCONTENT_DECLARE(
            peglib
            GIT_REPOSITORY https://github.com/yhirose/cpp-peglib
            GIT_TAG v1.9.1
            GIT_PROGRESS ${FOS_SHOW_GIT_PROGRESS})
    FETCHCONTENT_MAKEAVAILABLE(peglib)
    TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC peglib)
    INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/peglib-src/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/peglib)
    INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/peglib-src/ DESTINATION ${CMAKE_SOURCE_DIR}/include/peglib)
    TARGET_LINK_LIBRARIES(fhatos PRIVATE peglib)
ENDIF()
MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")

############################################################################################################
######################################## UNUSED LIBRARIES ##################################################
############################################################################################################

IF(DO_NOT)
    ####################################
    ####################################
    ### FMT: PRINT FORMATTING
    MESSAGE(CHECK_START "${.y}making fmt library${..}")
    FETCHCONTENT_DECLARE(
            fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt
            GIT_TAG 11.1.1
            GIT_PROGRESS ${FOS_SHOW_GIT_PROGRESS})
    FETCHCONTENT_MAKEAVAILABLE(fmt)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE fmt::fmt)
    INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/fmt-src/include/fmt/ DESTINATION ${CMAKE_BINARY_DIR}/include/fmt)
    FILE(COPY ${CMAKE_BINARY_DIR}/_deps/fmt-src/include/fmt/ DESTINATION ${CMAKE_BINARY_DIR}/include/fmt)
    TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/include/fmt)
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
    ####################################
    ####################################
    ### FTXUI: ANSI C++ TERMINAL UI
    MESSAGE(CHECK_START "${.y}making ftxui library${..}")
    SET(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
    FETCHCONTENT_DECLARE(
            ftxui
            GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
            GIT_TAG v5.0.0
            GIT_PROGRESS ${FOS_SHOW_GIT_PROGRESS})
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
############# TESTING ##############
####################################
IF(BUILD_TESTS)
    ADD_SUBDIRECTORY(test)
ENDIF()

####################################
########## DOCUMENTATION ###########
####################################
IF(BUILD_DOCS)
    ADD_SUBDIRECTORY(docs)
ENDIF()
