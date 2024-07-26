OPTION(BUILD_TESTS "Build tests" OFF) # ON/OFF
IF (BUILD_TESTS)
    MESSAGE("\n${.g}=====> ${.r}P${.y}R${.m}O${.y}C${.b}E${.m}S${.g}S${.c}I${.y}N${.r}G ${.m}T${.c}E${.g}S${.r}T${.y}S${..}\n")
    ENABLE_TESTING()
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
    FUNCTION(MAKE_TESTS PACKAGE PACKAGE_TESTS)
        SET(COUNTER 0)
        MESSAGE(STATUS "${.m}Processing " ${PACKAGE} " with " ${PACKAGE_TESTS}${..})
        FOREACH (PACKAGE_TEST ${PACKAGE_TESTS})
            MATH(EXPR COUNTER "${COUNTER}+1")
            MESSAGE(CHECK_START "${.y}CMaking ${PACKAGE_TEST}${..}")
            ADD_EXECUTABLE(${PACKAGE_TEST} ${CMAKE_SOURCE_DIR}/test/${PACKAGE}/${PACKAGE_TEST}/${PACKAGE_TEST}.cpp)
            ADD_DEFINITIONS("-DFOS_LOGGING=DEBUG" "-DNATIVE" "-Wno-ambiguous-reversed-operator") #  "-Wno-dev"
            TARGET_LINK_LIBRARIES(${PACKAGE_TEST} unity::framework tsl::ordered_map paho-mqttpp3)
            #TARGET_PRECOMPILE_HEADERS(${PACKAGE_TEST} PUBLIC ${ROOT_SOURCE_DIR}/test_fhatos.hpp)
            #TARGET_PRECOMPILE_HEADERS(${PACKAGE_TEST} REUSE_FROM ${PROJECT_NAME})
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
            SET_TESTS_PROPERTIES(${PACKAGE_TEST} PROPERTIES TIMEOUT 10)
            MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
        ENDFOREACH ()
        MATH(EXPR COUNTER "${COUNTER}+${TOTAL}")
        SET(TOTAL ${COUNTER} PARENT_SCOPE)
    ENDFUNCTION(MAKE_TESTS)
    ####################################
    ########### TEST SUITES ############
    ####################################
    MAKE_TESTS(language "test_fluent;test_obj;test_parser;test_insts")
    MAKE_TESTS(process "test_scheduler")
    MAKE_TESTS(process/actor "test_actor")
    MAKE_TESTS(process/router "test_router")
    MAKE_TESTS(structure "test_furi;test_uri")
    MAKE_TESTS(structure/io "test_fs")
    MAKE_TESTS(util "test_mutex")
    MESSAGE(STATUS "Total tests produced: ${.g}${TOTAL}${..}")
ENDIF ()