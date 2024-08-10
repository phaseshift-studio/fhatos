OPTION(BUILD_TESTS "Build tests" ON) # ON/OFF
IF (BUILD_TESTS)
    MESSAGE("\n${.g}=====> ${.r}P${.y}R${.m}O${.y}C${.b}E${.m}S${.g}S${.c}I${.y}N${.r}G ${.m}T${.c}E${.g}S${.r}T${.y}S${..}\n")
    ENABLE_TESTING()
    ####################################
    ########### BOOST TEST #############
    ####################################
    MESSAGE(CHECK_START "${.y}CMaking Boost Test Framework${..}")
    SET(PROJECT_UNIT_TESTS_NAME ${PROJECT_NAME}_boost_tests)
    FIND_PACKAGE(Boost COMPONENTS filesystem system unit_test_framework REQUIRED)
    FILE(GLOB UNIT_TESTS_SRC_FILES ${CMAKE_SOURCE_DIR}/test/structure/*_boost.cpp)
    SET(UNIT_TESTS_INCLUDES ${PROJECT_INCLUDE_DIR}
            ${CMAKE_SOURCE_DIR}/src
            ${UNIT_TESTS_SRC_FILES}
            ${Boost_INCLUDE_DIR})
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
    INCLUDE_DIRECTORIES(${UNIT_TESTS_INCLUDES})
    ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK)
    ADD_EXECUTABLE(${PROJECT_UNIT_TESTS_NAME} ${UNIT_TESTS_SRC_FILES})
    TARGET_LINK_LIBRARIES(${PROJECT_UNIT_TESTS_NAME} ${Boost_LIBRARIES})
    ADD_TEST(test_furi_boost ${PROJECT_UNIT_TESTS_NAME})
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
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
            ADD_DEFINITIONS("-DFOS_LOGGING=DEBUG" "-DNATIVE" "-Wno-ambiguous-reversed-operator") #  "-Wno-dev"
            TARGET_LINK_LIBRARIES(${PACKAGE_TEST} unity::framework tsl::ordered_map paho-mqttpp3)
            IF (USE_PRECOMPILED_HEADERS)
                MESSAGE("-- ${.m}${PACKAGE_TEST}${.y} using precompiled headers:${.g}${HEADER_FILES}")
                TARGET_PRECOMPILE_HEADERS(${PACKAGE_TEST} REUSE_FROM ${PROJECT_NAME})
            ENDIF ()
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
                SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -O1 -fno-omit-frame-pointer -g")
                TARGET_LINK_OPTIONS(${PACKAGE_TEST} BEFORE PUBLIC -fsanitize=undefined PUBLIC -fsanitize=address)
            ENDIF ()
            ADD_TEST(${PACKAGE_TEST} ${CMAKE_BINARY_DIR}/test/build/${PACKAGE_TEST}.out)
            SET_TESTS_PROPERTIES(${PACKAGE_TEST} PROPERTIES TIMEOUT 8)
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
    MAKE_TESTS(structure "test_furi" false)
    MAKE_TESTS(module "test_fs" true)
    MAKE_TESTS(util "test_mutex" true)
    MAKE_TESTS(play "play" false)
    MESSAGE(STATUS "Total tests produced: ${.g}${TOTAL}${..}")
ENDIF ()