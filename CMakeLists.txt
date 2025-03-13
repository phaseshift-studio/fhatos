CMAKE_MINIMUM_REQUIRED(VERSION 3.25.0)  # CMake version check
######## The fHatTy #########
PROJECT(fhatos) #############
#############################
###### CMAKE INCLUDES
INCLUDE(CMakePrintHelpers)
INCLUDE(FetchContent)
INCLUDE(ExternalProject)
###### CTEST INCLUDE ######
IF(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    INCLUDE(CTest)
ENDIF()
###### CPACK INCLUDE ######
IF(DO_NOT_USE)
    STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" is_top_level)
    OPTION(TOP_LEVEL_INCLUDE_PACKAGING "include project packaging rules" "${is_top_level}")
    IF(TOP_LEVEL_INCLUDE_PACKAGING)
        SET(CPACK_PACKAGING_INSTALL_PREFIX "/opt/${CPACK_PACKAGE_NAME}/${CMAKE_PROJECT_VERSION}")
        SET(CPACK_SET_DESTDIR ${CMAKE_BINARY_DIR})
        INCLUDE(CPack)
    ENDIF()
ENDIF()
##############################################
##############################################
##############################################

##############################################
############ RELEASE DEFINITIONS #############
##############################################
SET(FOS_NAME FhatOS)
SET(FOS_VERSION 0.1-alpha)
ADD_DEFINITIONS("-DFOS_NAME=${FOS_NAME}")
ADD_DEFINITIONS("-DFOS_VERSION=${FOS_VERSION}")
##############################################
##############################################
##############################################

##################################
## DEFINE MM-ADT ANSI COLOR DSL ##
##################################
IF(NOT WIN32)
    STRING(ASCII 27 Esc)
    SET(.. "${Esc}[m")
    SET(.r "${Esc}[31m")   # red
    SET(.g "${Esc}[32m")   # green
    SET(.y "${Esc}[33m")   # yellow
    SET(.b "${Esc}[34m")   # blue
    SET(.m "${Esc}[35m")   # magenta
    SET(.c "${Esc}[36m")   # cyan
    SET(.w "${Esc}[37m")   # white
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
IF(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)
    ADD_SUBDIRECTORY(src)
ENDIF()
#########################################
IF(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    SET(CMAKE_BUILD_TYPE Debug)
ENDIF()
#########################################
IF(NOT DEFINED CMAKE_BINARY_DIR)
    SET(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
ENDIF()
#########################################
IF(NOT DEFINED CMAKE_BUILD_PARALLEL_LEVEL)
    SET(CMAKE_BUILD_PARALLEL_LEVEL 8)
ENDIF()
#########################################
IF(NOT DEFINED FOS_SHOW_GIT_PROGRESS)
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
#######################################################################################################################
IF(CMAKE_BUILD_TYPE MATCHES "Debug" AND SANITIZER)
    SET(CMAKE_CXX_FLAGS "-fsanitize=address -O1 -fno-omit-frame-pointer -g ${CMAKE_CXX_FLAGS}")
ENDIF()
FIND_PROGRAM(CCACHE_FOUND ccache)
IF(CCACHE_FOUND)
    MESSAGE(NOTICE "${.y}ccache${..} found at ${CCACHE_FOUND}")
    OPTION(USE_CCACHE "use ccache" OFF)
ELSE()
    MESSAGE(WARNING "${.y}ccache${..} not found")
    SET(USE_CCACHE OFF)
ENDIF()
################################
SET(ENV{no-dev})
#SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_C_COMPILER_VERSION 14)
SET(CMAKE_CXX_COMPILER_VERSION 14)
SET(CMAKE_CXX_FLAGS "-Wall -Wextra ${CMAKE_CXX_FLAGS}")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -lbfd -ldl ${CMAKE_CXX_FLAGS_DEBUG}")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 ${CMAKE_CXX_FLAGS_RELEASE}")
SET(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "include-what-you-use")
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
OPTION(FOS_SHOW_GIT_PROGRESS "verbose git output on fetch content" ON)
MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
####################################################################
######################## CTEST TESTING #############################
####################################################################
IF(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTS)
    SET(BUILD_TESTING ON)
    SET(PRINT_STACKTRACE ON)
ENDIF()
####################################################################
####################### CPACK PACKAGING ############################
####################################################################
MESSAGE(CHECK_START "${.y}configuring packaging with cpack${..}")
IF(DO_NOT_USE)
    IF(TOP_LEVEL_INCLUDE_PACKAGING)
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
    ELSE()
        MESSAGE(NOTICE "${PROJECT_NAME} is not the ${.r}top level project${..}. Using enclosing project's ${.y}CPack${..} configurations.")
    ENDIF()
ENDIF()
MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
####################################################################
SET(FOS_MACHINE_SUBOS ${CMAKE_SYSTEM})
SET(FOS_MACHINE_ARCH ${CMAKE_SYSTEM_PROCESSOR})
MESSAGE(NOTICE "
\t  \t${.m}-----${.y}Co${.r}M${.b}m${.c}on ${.r}b${.m}U${.c}il${.b}D ${.r}Pr${.m}op${.y}eR${.g}Ti${.r}Es${.m}-----${..}
\t  ${.g}PROJECT_NAME${..}                  : ${PROJECT_NAME}
\t  ${.g}CMAKE_BUILD_TYPE${..}              : ${CMAKE_BUILD_TYPE}
\t  ${.g}CMAKE_SOURCE_DIR${..}              : ${CMAKE_SOURCE_DIR}
\t  ${.g}CMAKE_BINARY_DIR${..}              : ${CMAKE_BINARY_DIR}
\t  ${.g}FOS_SKIP_CMAKE${..}                : ${FOS_SKIP_CMAKE}
\t  ${.g}FOS_LOGGING${..}                   : ${FOS_LOGGING}
\t  ${.g}CHECK_INTERNET${..}                : ${CHECK_INTERNET}
\t  ${.g}BUILD_TESTS${..}                   : ${BUILD_TESTS}
\t  ${.g}BUILD_DOCS${..}                    : ${BUILD_DOCS}
\t  ${.g}SANITIZER${..}                     : ${SANITIZER}
\t  ${.g}USE_CCACHE${..}                    : ${USE_CCACHE}
\t  ${.g}CMAKE_CXX_INCLUDE_WHAT_YOU_USE${..}: ${CMAKE_CXX_INCLUDE_WHAT_YOU_USE}
\t  ${.g}PRINT_STACKTRACE${..}              : ${PRINT_STACKTRACE}

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

OPTION(BUILD_NATIVE "build native" ON) # ON/OFF
IF(BUILD_NATIVE)
    #####################################################################################
    #####################################################################################
    ######################### FHATOS TARGET (EXECUTABLE) ################################
    #####################################################################################
    #####################################################################################
    INCLUDE("${PROJECT_SOURCE_DIR}/cmake/fhatos_functions.cmake")
    ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/extern/fmt)
    ADD_EXECUTABLE(${PROJECT_NAME} "src/main.cpp" ${HEADERS})
    CREATE_TARGET(${PROJECT_NAME})
    #####################################################################################
    #####################################################################################
    #####################################################################################
    #####################################################################################
ENDIF()

####################################
############# TESTING ##############
####################################
IF(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTS)
    INCLUDE_DIRECTORIES(src)
    ADD_SUBDIRECTORY(test)
ENDIF()

####################################
########## DOCUMENTATION ###########
####################################
IF(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_DOCS)
    ADD_SUBDIRECTORY(docs)
ENDIF()
