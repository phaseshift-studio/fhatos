OPTION(BUILD_TESTS "Build tests" ON) # ON/OFF
SET(CMAKE_MACOSX_RPATH 1)
IF (BUILD_TESTS)
    MESSAGE("\n${.g}=====> ${.r}P${.y}R${.m}O${.y}C${.b}E${.m}S${.g}S${.c}I${.y}N${.r}G ${.m}T${.c}E${.g}S${.r}T${.y}S${..}\n")
    ENABLE_TESTING()
    MESSAGE(CHECK_START "${.y}Searching for MQTT Broker${..}")
    SET(MQTT_BROKER_PORT 1883)
    EXECUTE_PROCESS(
            COMMAND netstat -ant | grep ${MQTT_BROKER_PORT}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            OUTPUT_VARIABLE port_info
            ERROR_VARIABLE port_error
    )
    STRING(REGEX MATCHALL ".${MQTT_BROKER_PORT}" BUILD_MQTT_TESTS ${port_info})
    IF (BUILD_MQTT_TESTS)
        MESSAGE(STATUS "${.m}Will build MQTT tests as port ${.y}${MQTT_BROKER_PORT}${.m} is ${.g}available${..}")
    ELSE ()
        MESSAGE(STATUS "${.m}Will not build MQTT tests as port ${.y}${MQTT_BROKER_PORT}${.m} is ${.r}unavailable${..}")
    ENDIF ()
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
    ####################################
    ########### BOOST TEST #############
    ####################################
    OPTION(BUILD_BOOST_TESTS "Build Boost tests" OFF) # ON/OFF
    IF (BUILD_BOOST_TESTS)
        MESSAGE(CHECK_START "${.y}CMaking Boost Test Framework${..}")
        SET(PROJECT_UNIT_TESTS_NAME ${PROJECT_NAME}_boost_tests)
        CMAKE_POLICY(SET CMP0167 NEW)
        SET(Boost_USE_STATIC_LIBS OFF)
        SET(Boost_USE_MULTITHREADED ON)
        SET(Boost_USE_STATIC_RUNTIME OFF)
        FIND_PACKAGE(Boost COMPONENTS filesystem system unit_test_framework REQUIRED)
        IF (Boost_FOUND)
            FILE(GLOB UNIT_TESTS_SRC_FILES ${CMAKE_SOURCE_DIR}/test/structure/*_boost.cpp)
            SET(UNIT_TESTS_INCLUDES ${PROJECT_INCLUDE_DIR}
                    ${CMAKE_SOURCE_DIR}/src
                    ${Boost_INCLUDE_DIR})
            INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
            INCLUDE_DIRECTORIES(${UNIT_TESTS_INCLUDES})
            ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK -DBoost_NO_BOOST_CMAKE=TRUE)
            ADD_EXECUTABLE(${PROJECT_UNIT_TESTS_NAME} ${UNIT_TESTS_SRC_FILES})
            TARGET_LINK_LIBRARIES(${PROJECT_UNIT_TESTS_NAME} ${Boost_LIBRARIES} ordered_map paho-mqttpp3)
            ADD_TEST(test_furi_boost ${PROJECT_UNIT_TESTS_NAME})
        ELSE ()
            MESSAGE("${.r}Unable to locate Boost unit_test_framework${..}. Proceeding without respective depending tests.")
        ENDIF ()
        MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
    ENDIF ()
    ####################################
    ######## EXTERNAL LIBRARIES ########
    ####################################
    MESSAGE(CHECK_START "${.y}CMaking Unity Test Framework${..}")
    FETCHCONTENT_DECLARE(
            unity
            GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
            GIT_TAG v2.6.0
    )
    FETCHCONTENT_MAKEAVAILABLE(unity)
    INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/unity-src/src/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/unity)
    INSTALL(DIRECTORY ${CMAKE_BINARY_DIR}/_deps/unity-src/src/ DESTINATION ${CMAKE_SOURCE_DIR}/include/unity)
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
    ###########################################
    ########### TEST SUITE BUILDER ############
    ###########################################
    SET(TOTAL 0)
    FUNCTION(MAKE_TESTS PACKAGE PACKAGE_TESTS USE_PRECOMPILED_HEADERS)
        SET(COUNTER 0)
        MESSAGE(STATUS "${.m}Processing " ${PACKAGE} " with " ${PACKAGE_TESTS}${..})
        FOREACH (PACKAGE_TEST ${PACKAGE_TESTS})
            MATH(EXPR COUNTER "${COUNTER}+1")
            MESSAGE(CHECK_START "${.y}CMaking ${PACKAGE_TEST}${..}")
            ADD_EXECUTABLE(${PACKAGE_TEST} ${CMAKE_SOURCE_DIR}/test/${PACKAGE}/${PACKAGE_TEST}/${PACKAGE_TEST}.cpp)
            ADD_DEFINITIONS("-DFOS_LOGGING=DEBUG" "-DNATIVE")
            IF (APPLE)
                ADD_COMPILE_OPTIONS("-Wno-ambiguous-reversed-operator") # a=b potentially not equivalent to b=a (operator overloading of =)
            ENDIF ()
            IF (USE_PRECOMPILED_HEADERS)
                MESSAGE("-- ${.m}${PACKAGE_TEST}${.y} using precompiled headers:${.g}${HEADER_FILES}")
                TARGET_PRECOMPILE_HEADERS(${PACKAGE_TEST} REUSE_FROM ${PROJECT_NAME})
            ENDIF ()
            TARGET_LINK_LIBRARIES(${PACKAGE_TEST} unity ordered_map paho-mqttpp3)
            SET_TARGET_PROPERTIES(${PACKAGE_TEST} PROPERTIES
                    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test/build"
                    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test/build"
                    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test/build"
                    OUTPUT_NAME ${PACKAGE_TEST}
                    SUFFIX ".out"
                    # UNITY_BUILD ON
                    UNITY_BUILD_MODE BATCH
                    UNITY_BUILD_BATCH_SIZE 16)
            IF (CMAKE_BUILD_TYPE MATCHES "DEBUG")
                #TARGET_COMPILE_OPTIONS(${PACKAGE_TEST} PUBLIC -fsanitize=address)
                #TARGET_LINK_OPTIONS(${PACKAGE_TEST} PUBLIC -fsanitize=address)
                #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -O1 -fno-omit-frame-pointer -g")
                #TARGET_LINK_OPTIONS(${PACKAGE_TEST} BEFORE PUBLIC -fsanitize=undefined PUBLIC -fsanitize=address)
            ENDIF ()
            ADD_TEST(${PACKAGE_TEST} ${CMAKE_BINARY_DIR}/test/build/${PACKAGE_TEST}.out)
            SET_TESTS_PROPERTIES(${PACKAGE_TEST} PROPERTIES TIMEOUT 20)
            MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
        ENDFOREACH ()
        MATH(EXPR COUNTER "${COUNTER}+${TOTAL}")
        SET(TOTAL ${COUNTER} PARENT_SCOPE)
    ENDFUNCTION(MAKE_TESTS)
    ####################################
    ########### TEST SUITES ############
    ####################################
    MAKE_TESTS(language "test_fluent;test_obj;test_parser;test_insts" true)
    MAKE_TESTS(process "test_scheduler" true)
    MAKE_TESTS(process/actor "test_actor" true)
    MAKE_TESTS(structure/router "test_router" true)
    MAKE_TESTS(structure/stype/key_value "test_key_value" , true)
    IF (BUILD_MQTT_TESTS)
        MAKE_TESTS(structure/stype/mqtt "test_mqtt" , true)
    ENDIF ()
    MAKE_TESTS(. "test_main;test_furi" false)
    MAKE_TESTS(model "test_fs" true)
    MAKE_TESTS(util "test_mutex" true)
    MESSAGE(STATUS "Total tests produced: ${.g}${TOTAL}${..}")
ENDIF ()