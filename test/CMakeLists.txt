PROJECT(fhatos-test)
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
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/_deps/unity-src/include/unity/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include/unity)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/_deps/unity-src/include/unity/ DESTINATION ${CMAKE_SOURCE_DIR}/include/unity)
MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
####################################
########### TEST SUITES ############
####################################
SET(PACKAGE language)
SET(PACKAGE_TESTS "test_binary_obj;test_fluent;test_obj;test_parser")
FOREACH (PACKAGE_TEST ${PACKAGE_TESTS})
    MESSAGE(CHECK_START "${.y}CMaking ${PACKAGE_TEST}${..}")
    ADD_EXECUTABLE(${PACKAGE_TEST} ${CMAKE_SOURCE_DIR}/test/${PACKAGE}/${PACKAGE_TEST}/${PACKAGE_TEST}.cpp)
    ADD_DEFINITIONS("-DNATIVE")
    SET_TARGET_PROPERTIES(${PACKAGE_TEST} PROPERTIES
            OUTPUT_NAME ${PACKAGE_TEST}
            SUFFIX ".out"
            UNITY_BUILD ON
            UNITY_BUILD_MODE BATCH
            UNITY_BUILD_BATCH_SIZE 16)
    TARGET_LINK_LIBRARIES(${PACKAGE_TEST} unity::framework tsl::ordered_map)
    ADD_TEST(${PACKAGE_TEST} ${CMAKE_SOURCE_DIR}/test/${PACKAGE}/${PACKAGE_TEST}/${PACKAGE_TEST}.cpp)
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
ENDFOREACH ()

