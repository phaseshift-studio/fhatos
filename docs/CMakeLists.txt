FIND_PACKAGE(Python3 COMPONENTS Interpreter)
OPTION(BUILD_DOCS "Build documentation" ON) # ON/OFF
SET(CMAKE_MACOSX_RPATH 1)
IF(TASKSCH_BUILD_WITH_DOCS)
    ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/docs)
ENDIF()
IF(BUILD_DOCS)
    MESSAGE("\n${.g}=====> ${.r}P${.y}R${.m}O${.y}C${.b}E${.m}S${.g}S${.c}I${.y}N${.r}G ${.m}D${.c}O${.g}C${.r}S${..}\n")
    SET(TOTAL 0)
    FUNCTION(MAKE_CODE_RUNNERS PACKAGE PACKAGE_RUNNERS)
        SET(COUNTER 0)
        MESSAGE(STATUS "${.m}Processing " ${PACKAGE} " with " ${PACKAGE_RUNNERS}${..})
        FOREACH(PACKAGE_RUNNER ${PACKAGE_RUNNERS})
            MATH(EXPR COUNTER "${COUNTER}+1")
            MESSAGE(CHECK_START "${.y}CMaking ${PACKAGE_RUNNER}${..}")
            ADD_DEFINITIONS(
                    -DNATIVE
                    -DBUILD_DOCS=ON)
            ADD_EXECUTABLE(${PACKAGE_RUNNER} ${CMAKE_SOURCE_DIR}/docs/src/${PACKAGE}/${PACKAGE_RUNNER}.cpp)
            IF(USE_PAHO_MQTT_HACK)
                MESSAGE(CHECK_START "${.r}Performing Paho-MQTT Build Hack ${.g}[${.y}TEST SUITE${.g}]${..}")
                TARGET_LINK_LIBRARIES(${PACKAGE_RUNNER} PUBLIC
                        ${CMAKE_BINARY_DIR}/include/mqtt-c/libpaho-mqtt3c.so.1.3.13
                        ${CMAKE_BINARY_DIR}/include/mqtt-c/libpaho-mqtt3a.so.1.3.13
                        ${CMAKE_BINARY_DIR}/include/mqtt-c/libpaho-mqtt3a.so.1
                        paho-mqttpp3)
                MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
            ELSE()
                TARGET_LINK_LIBRARIES(${PACKAGE_RUNNER} paho-mqttpp3)
            ENDIF()
            TARGET_PRECOMPILE_HEADERS(${PACKAGE_RUNNER} REUSE_FROM ${PROJECT_NAME})
            SET_TARGET_PROPERTIES(${PACKAGE_RUNNER} PROPERTIES
                    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/docs/build"
                    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/docs/build"
                    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/docs/build"
                    OUTPUT_NAME ${PACKAGE_RUNNER}
                    SUFFIX ".out")
            MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
        ENDFOREACH()
        MATH(EXPR COUNTER "${COUNTER}+${TOTAL}")
        SET(TOTAL ${COUNTER} PARENT_SCOPE)
    ENDFUNCTION(MAKE_CODE_RUNNERS)
    MAKE_CODE_RUNNERS(runner main_runner)
    MAKE_CODE_RUNNERS(runner boot_runner)
    MESSAGE(STATUS "Total runners produced: ${.g}${TOTAL}${..}")
    #####################################################################
    MESSAGE(CHECK_START "${.y}CMaking AsciiDoc Documentation${..}")
    ADD_CUSTOM_TARGET(asciidoc ALL DEPENDS main_runner boot_runner)
    SET(FILES ${CMAKE_SOURCE_DIR}/docs/fhatos.adoc)
    ADD_CUSTOM_COMMAND(TARGET asciidoc
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMAND markdown-code-runner --verbose ${FILES}
            COMMENT "Evaluating markdown-code-runner: ${FILES}")
    ADD_CUSTOM_COMMAND(TARGET asciidoc
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/docs
            COMMAND asciidoctor -o index.html ${FILES}
            COMMENT "Evaluating asciidoctor: ${FILES}")
    UNSET(FILES)
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
    #####################################################################
ENDIF()