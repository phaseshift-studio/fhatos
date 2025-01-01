    IF(APPLE)
        SET(CMAKE_MACOSX_RPATH 1)
    ENDIF()
    IF(BUILD_TESTS)
        MESSAGE("\n${.g}=====> ${.r}P${.y}R${.m}O${.y}C${.b}E${.m}S${.g}S${.c}I${.y}N${.r}G ${.m}T${.c}E${.g}S${.r}T${.y}S${..}\n")
        ENABLE_TESTING()
        MESSAGE(CHECK_START "${.y}searching for MQTT broker${..}")
        SET(MQTT_BROKER_PORT 1883)
        EXECUTE_PROCESS(
                COMMAND netstat -ant | grep ${MQTT_BROKER_PORT}
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                OUTPUT_VARIABLE port_info
                ERROR_VARIABLE port_error
        )
        STRING(REGEX MATCH ".${MQTT_BROKER_PORT}" BUILD_MQTT_TESTS ${port_info} NOTHING NOTHING)
        IF(BUILD_MQTT_TESTS)
            MESSAGE(STATUS "${.m}Will build MQTT tests as port ${.y}${MQTT_BROKER_PORT}${.m} is ${.g}available${..}")
        ELSE()
            MESSAGE(STATUS "${.m}Will not build MQTT tests as port ${.y}${MQTT_BROKER_PORT}${.m} is ${.r}unavailable${..}")
        ENDIF()
        MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
        ####################################
        ########### BOOST TEST #############
        ####################################
        OPTION(BUILD_BOOST_TESTS "build boost tests" OFF) # ON/OFF
        IF(BUILD_BOOST_TESTS)
            MESSAGE(CHECK_START "${.y}making boost test framework${..}")
            SET(PROJECT_UNIT_TESTS_NAME ${PROJECT_NAME}_boost_tests)
            #CMAKE_POLICY(SET CMP0167 NEW)
            SET(Boost_USE_STATIC_LIBS OFF)
            SET(Boost_USE_MULTITHREADED ON)
            SET(Boost_USE_STATIC_RUNTIME OFF)
            FIND_PACKAGE(Boost COMPONENTS unit_test_framework REQUIRED)
            IF(Boost_FOUND)
                FILE(GLOB UNIT_TESTS_SRC_FILES ${CMAKE_SOURCE_DIR}/test/structure/*_boost.cpp)
                SET(UNIT_TESTS_INCLUDES ${PROJECT_INCLUDE_DIR}
                        ${CMAKE_SOURCE_DIR}/src
                        ${Boost_INCLUDE_DIR})
                INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
                INCLUDE_DIRECTORIES(${UNIT_TESTS_INCLUDES})
                ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK -DBoost_NO_BOOST_CMAKE=TRUE)
                ADD_EXECUTABLE(${PROJECT_UNIT_TESTS_NAME} ${UNIT_TESTS_SRC_FILES})
                TARGET_LINK_LIBRARIES(${PROJECT_UNIT_TESTS_NAME} ${Boost_LIBRARIES} paho-mqtt3c paho-mqttpp3 ordered_map)
                ADD_TEST(test_furi_boost ${PROJECT_UNIT_TESTS_NAME})
            ELSE()
                MESSAGE("${.r}unable to locate boost unit_test_framework${..}. proceeding without respective depending tests.")
            ENDIF()
            MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
        ENDIF()
        ####################################
        ######## EXTERNAL LIBRARIES ########
        ####################################
        MESSAGE(CHECK_START "${.y}making unity test framework${..}")
        FETCHCONTENT_DECLARE(
                unity
                GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
                GIT_TAG v2.6.0
                GIT_PROGRESS ${FOS_SHOW_GIT_PROGRESS}
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
            MESSAGE(STATUS "${.m}Processing ${.g}${PACKAGE}${.m} with ${PACKAGE_TESTS}${..}")
            FOREACH(PACKAGE_TEST ${PACKAGE_TESTS})
                IF(NOT PACKAGE_TEST IN_LIST BUILD_TESTS AND NOT ON IN_LIST BUILD_TESTS)
                    MESSAGE(STATUS "\t${.r}ignoring ${.y}${PACKAGE_TEST} ${.r}as${..} ${.g}BUILD_TESTS${..}=${BUILD_TESTS}")
                ELSE()
                    MATH(EXPR COUNTER "${COUNTER}+1")
                    MESSAGE(CHECK_START "${.y}making ${PACKAGE_TEST}${..}")
                    ADD_EXECUTABLE(${PACKAGE_TEST} ${CMAKE_SOURCE_DIR}/test/${PACKAGE}/${PACKAGE_TEST}/${PACKAGE_TEST}.cpp)
                    ADD_DEFINITIONS(
                            -DNATIVE
                            -DFOS_LOGGING=${FOS_LOGGING})
                    IF(APPLE)
                        ADD_COMPILE_OPTIONS("-Wno-ambiguous-reversed-operator") # a=b potentially not equivalent to b=a (operator overloading of =)
                    ENDIF()
                    IF(USE_PRECOMPILED_HEADERS)
                        MESSAGE("-- ${.m}${PACKAGE_TEST}${.y} using precompiled headers:${.g}${HEADER_FILES}${..}")
                        TARGET_PRECOMPILE_HEADERS(${PACKAGE_TEST} REUSE_FROM ${PROJECT_NAME})
                    ENDIF()
                    IF(USE_PAHO_MQTT_HACK)
                        MESSAGE(CHECK_START "${.r}performing paho-MQTT build hack ${.g}[${.y}TEST SUITE${.g}]${..}")
                        TARGET_LINK_LIBRARIES(${PACKAGE_TEST} PUBLIC
                                ${CMAKE_BINARY_DIR}/include/mqtt-c/libpaho-mqtt3c.so.1.3.13
                                ${CMAKE_BINARY_DIR}/include/mqtt-c/libpaho-mqtt3a.so.1.3.13
                                ${CMAKE_BINARY_DIR}/include/mqtt-c/libpaho-mqtt3a.so.1
                                unity paho-mqttpp3)
                        MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
                    ELSE()
                        TARGET_LINK_LIBRARIES(${PACKAGE_TEST} unity paho-mqttpp3 peglib)
                    ENDIF()
                    SET_TARGET_PROPERTIES(${PACKAGE_TEST} PROPERTIES
                            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test/build"
                            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test/build"
                            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test/build"
                            OUTPUT_NAME ${PACKAGE_TEST}
                            SUFFIX ".out"
                            # UNITY_BUILD ON
                            UNITY_BUILD_MODE BATCH
                            UNITY_BUILD_BATCH_SIZE 16)
                    STRING(TOLOWER CMAKE_BUILD_TYPE cmake_build_type_lower)
                    IF(SANITIZER AND cmake_build_type_lower STREQUAL "debug")
                        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -O1 -fno-omit-frame-pointer -g")
                        MESSAGE(NOTICE "  ${.m}.${.r}-${.y}= ${.g}using address sanitizer${.c} =${.r}-${.m}.${..}\n")
                    ELSE()
                        MESSAGE(NOTICE "  ${.m}.${.g}-${.y}= ${.r}NOT using address sanitizer${.y} =${.g}-${.m}.${..}\n")
                    ENDIF()
                    ADD_TEST(${PACKAGE_TEST} ${CMAKE_BINARY_DIR}/test/build/${PACKAGE_TEST}.out)
                    SET_PROPERTY(TEST ${PACKAGE_TEST} PROPERTY TIMEOUT 20)
                    SET_PROPERTY(TEST ${PACKAGE_TEST} PROPERTY FAIL_REGULAR_EXPRESSION "${PACKAGE_TEST}(.cpp|.hpp):[0-9]+:[_a-z]+:FAIL") # test_process.cpp:73:test_thread:FAIL
                    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
                ENDIF()
            ENDFOREACH()
            MATH(EXPR COUNTER "${COUNTER}+${TOTAL}")
            SET(TOTAL ${COUNTER} PARENT_SCOPE)
        ENDFUNCTION(MAKE_TESTS)
        ####################################
        ########### TEST SUITES ############
        ####################################
        IF(PLAYTIME)
            MAKE_TESTS(play "play" , true)
        ELSE()
           # MAKE_TESTS(language "test_obj" true)
          #  MAKE_TESTS(language/mmadt "test_mmadt_parser;test_mmadt_inst" true)
            MAKE_TESTS(language/processor "test_processor" true)
            #  MAKE_TESTS(process "test_scheduler;test_process" true)
          #  MAKE_TESTS(structure "test_structure" true)
            #  MAKE_TESTS(structure/router "test_router" true)
            #  MAKE_TESTS(structure/stype/key_value "test_heap" , true)
            #  IF(BUILD_MQTT_TESTS)
            #      MAKE_TESTS(structure/stype/mqtt "test_mqtt" , true)
            #  ENDIF()
          #  MAKE_TESTS(. "test_main;test_furi" false)
            #  MAKE_TESTS(model "test_fs" true)
            #  MAKE_TESTS(util "test_mutex;test_string_helper" true)
        ENDIF()
        MESSAGE(STATUS "${.g}total tests produced${..}: ${.y}${TOTAL}${..}")
    ENDIF()
    MESSAGE(STATUS,"${.g}tests processed${..}")