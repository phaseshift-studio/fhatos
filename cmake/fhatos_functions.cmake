FUNCTION(CREATE_TARGET TARGET_NAME)
    FIND_PROGRAM(IWYU_PATH NAMES include-what-you-use include-what-you-use REQUIRED)
    IF(NOT IWYU_PATH)
        MESSAGE(FATAL_ERROR "Could not find the program include-what-you-use")
    ENDIF()
    ########################################################
    IF(APPLE)
        SET(CMAKE_MACOSX_RPATH 1)
    ENDIF()
    ########################################################
    MESSAGE("\n${.g}=====> ${.r}P${.y}R${.m}O${.y}C${.b}E${.m}S${.g}S${.c}I${.y}N${.r}G ${.m}N${.c}A${.g}T${.r}I${.y}V${.b}E${..}\n")
    MESSAGE(CHECK_START "${.y}making ${TARGET_NAME} (${.r}${PLATFORM}${..}${.y})${..} (${.y}-D${.g}NATIVE${..})")
    MESSAGE(STATUS "${.y}build type${..}: ${.g}${CMAKE_BUILD_TYPE}${..}")

    ############# IGNORED SOURCE/HEADER FILES ##############
    FILE(GLOB_RECURSE TO_REMOVE RELATIVE "${CMAKE_SOURCE_DIR}" src/model/ui/*.* extern/fmt-arduino-10.1.1/*.*)
    MESSAGE(STATUS "${.y}ignoring files${..}: ${.g}${TO_REMOVE}${..}")
    ########################################################
    FILE(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} src/*.cpp)
    MESSAGE(NOTICE ${SOURCES})
    LIST(REMOVE_ITEM SOURCES ${TO_REMOVE})
    MESSAGE(STATUS "${.y}adding source files${..}: ${.g}${SOURCES}${..}")
    FILE(GLOB_RECURSE HEADERS RELATIVE ${CMAKE_SOURCE_DIR} src/*.hpp)
    LIST(REMOVE_ITEM HEADERS ${TO_REMOVE})
    MESSAGE(STATUS "${.y}adding header files${..}: ${.g}${HEADERS}${..}")
    IF(USE_CCACHE)
        SET(CMAKE_CXX_COMPILER_LAUNCHER ccache)
    ENDIF()

    ########################################################
    SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES
            # CXX_INCLUDE_WHAT_YOU_USE "${IWYU_PATH}"
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
            OUTPUT_NAME ${TARGET_NAME}
            SUFFIX ".out"
    )
    TARGET_SOURCES(${TARGET_NAME} PRIVATE ${SOURCES})
    TARGET_COMPILE_FEATURES(${TARGET_NAME} PRIVATE cxx_std_17)
    ##### PRECOMPILED HEADERS #####
    SET(ROOT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
    STRING(CONCAT HEADERS
            "<any>;<atomic>;<deque>;<functional>;<vector>;<map>;<memory>;<optional>;"
            "<queue>;<map>;<set>;<string>;<ostream>;<shared_mutex>;<utility>;<variant>;<gpiod.h>")
    MESSAGE(STATUS "${.y}processing precompiled headers: ${.g}${HEADERS}${..}")
    TARGET_PRECOMPILE_HEADERS(${TARGET_NAME} PUBLIC ${HEADERS})
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
    FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/fs) # file system root for executable
    FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/boot) # file system boot for executable
    FILE(COPY_FILE "${CMAKE_SOURCE_DIR}/conf/boot_config.obj" "${CMAKE_BINARY_DIR}/boot/boot_config.obj")
    FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/include)
    INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/include)
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")

    ##############################################################################################################
    ##############################################################################################################
    ##############################################################################################################

    ####################################
    ######## EXTERNAL LIBRARIES ########
    ####################################
    SET(TARGET_LIBRARIES "${PROJECT_NAME}-lib" paho-mqttpp3 paho-mqtt3a fmt::fmt cpptrace::cpptrace PARENT_SCOPE)
    ###############################################################################################
    ####################################
    ####################################
    ### CPPTRACE: STACKTRACE UTILITY
    MESSAGE(CHECK_START "${.y}making cpptrace library${..}")
    FETCHCONTENT_DECLARE(
            cpptrace
            GIT_REPOSITORY https://github.com/jeremy-rifkin/cpptrace.git
            GIT_TAG v0.8.2
    )
    FETCHCONTENT_MAKEAVAILABLE(cpptrace)
    TARGET_LINK_LIBRARIES(${TARGET_NAME} PUBLIC cpptrace::cpptrace)
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
    ####################################
    ####################################
    ### GPIOD: GPIO READ/WRITE ACCESS
    MESSAGE(CHECK_START "${.y}making libgpiod library${..}")
    FIND_LIBRARY(GPIOD_LIBRARY NAMES libgpiod.so)
    IF(NOT GPIOD_LIBRARY)
        MESSAGE(CHECK_FAIL "[${.g}ERROR${..}]")
        MESSAGE(FATAL_ERROR "gpiod library not found. Install apt install libgpiod-dev")
    ELSE()
        TARGET_LINK_LIBRARIES(${TARGET_NAME} PRIVATE ${GPIOD_LIBRARY})
        MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
    ENDIF()
    ####################################
    ####################################
    ### FMT: STRING AND PRINT FORMATTING
    MESSAGE(CHECK_START "${.y}making fmt library${..}")
    MESSAGE(NOTICE "\t${.r}IMPORTANT${..}: ${.m}fmt${..} distributed w/ ${.FHATOS} via ${.b}include/fmt${..}.")
    INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/extern/fmt/include/fmt/ DESTINATION ${CMAKE_BINARY_DIR}/include/fmt)
    FILE(COPY ${CMAKE_SOURCE_DIR}/extern/fmt/src/ DESTINATION ${CMAKE_BINARY_DIR}/include/fmt)
    TARGET_INCLUDE_DIRECTORIES(${TARGET_NAME} PRIVATE ${CMAKE_BINARY_DIR}/include/fmt)
    TARGET_LINK_LIBRARIES(${TARGET_NAME} PRIVATE fmt::fmt)
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
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
        TARGET_INCLUDE_DIRECTORIES(${TARGET_NAME} PRIVATE ${CMAKE_BINARY_DIR}/include/tsl)
        TARGET_LINK_LIBRARIES(${TARGET_NAME} PRIVATE ordered_map)
    ENDIF()
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
    ####################################
    ####################################
    ### MQTT: MQTT NETWORK PROTOCOL IMPLEMENTATION
    ####################################
    MESSAGE(CHECK_START "${.y}making paho mqtt C++ library${..}")
    FETCHCONTENT_DECLARE(
            paho-mqttpp3
            GIT_REPOSITORY https://github.com/eclipse/paho.mqtt.cpp.git
            GIT_TAG v1.4.1
            GIT_PROGRESS ${FOS_SHOW_GIT_PROGRESS})
    ##################################################################
    ##### HACK TO FIX LIBRARY NAMING ERRORS IN PAHO MQTT RELEASE #####
    ##################################################################
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
    ##################################################################
    ##################################################################
    ##################################################################
    FETCHCONTENT_MAKEAVAILABLE(paho-mqttpp3)
    INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/paho-mqtt3c-build/src DESTINATION ${CMAKE_INSTALL_PREFIX}/include/mqtt-c/src)
    INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/paho-mqttpp3-src/include DESTINATION ${CMAKE_INSTALL_PREFIX}/include/mqtt-cpp)
    MESSAGE(CHECK_START "${.r}performing paho-mqtt build hack ${.g}[${.y}PART DEUX${.g}]${..}")
    #FILE(COPY ${CMAKE_BINARY_DIR}/_deps/paho-mqttpp3-src/include/ DESTINATION ${CMAKE_BINARY_DIR}/include/mqtt-cpp)
    #FILE(COPY ${CMAKE_BINARY_DIR}/_deps/paho-mqtt3c-src/src/ DESTINATION ${CMAKE_BINARY_DIR}/include/mqtt-c)
    TARGET_INCLUDE_DIRECTORIES(${TARGET_NAME} PRIVATE ${CMAKE_BINARY_DIR}/include/mqtt-c)
    TARGET_INCLUDE_DIRECTORIES(${TARGET_NAME} PRIVATE ${CMAKE_BINARY_DIR}/include/mqtt-c/src/)
    TARGET_INCLUDE_DIRECTORIES(${TARGET_NAME} PRIVATE ${CMAKE_BINARY_DIR}/include/mqtt-cpp)
    TARGET_LINK_LIBRARIES(${TARGET_NAME} PRIVATE paho-mqtt3a paho-mqttpp3)
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
    TARGET_LINK_DIRECTORIES(${TARGET_NAME} PRIVATE ${CMAKE_BINARY_DIR}/include/mqtt-c)
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
        TARGET_INCLUDE_DIRECTORIES(${TARGET_NAME} PUBLIC ${WIRINGXX_LOCATION}/wiring${XX})
        TARGET_LINK_LIBRARIES(${TARGET_NAME} PUBLIC ${WIRINGXX_LIBRARIES})
        MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
    ENDIF()
    ####################################
    ####################################
    ### PEGLIB: A C++ PEG PARSER
    MESSAGE(CHECK_START "${.y}making cpp-peglib library${..}")
    MESSAGE(NOTICE "\t${.r}IMPORTANT${..}: ${.m}cpp-peglib${..} distributed w/ ${.FHATOS} via ${.b}language/util/peglib.h${..} (altered for microprocessor architectures).")
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
ENDFUNCTION()