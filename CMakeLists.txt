FIND_PACKAGE(PythonInterp REQUIRED)
OPTION(BUILD_DOCS "Build documentation" ON) # ON/OFF
IF (TASKSCH_BUILD_WITH_DOCS)
    ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/docs)
ENDIF ()
# IF (BUILD_DOCS )
MESSAGE("\n${.g}=====> ${.r}P${.y}R${.m}O${.y}C${.b}E${.m}S${.g}S${.c}I${.y}N${.r}G ${.m}D${.c}O${.g}C${.r}S${..}\n")
SET(TOTAL 0)
FUNCTION(MAKE_CODE_RUNNERS PACKAGE PACKAGE_RUNNERS)
    SET(COUNTER 0)
    MESSAGE(STATUS "${.m}Processing " ${PACKAGE} " with " ${PACKAGE_RUNNERS}${..})
    FOREACH (PACKAGE_RUNNER ${PACKAGE_RUNNERS})
        MATH(EXPR COUNTER "${COUNTER}+1")
        MESSAGE(CHECK_START "${.y}CMaking ${PACKAGE_RUNNER}${..}")
        ADD_EXECUTABLE(${PACKAGE_RUNNER} ${CMAKE_SOURCE_DIR}/docs/src/${PACKAGE}/${PACKAGE_RUNNER}/${PACKAGE_RUNNER}.cpp)
        ADD_DEFINITIONS("-DNATIVE" "-Wno-ambiguous-reversed-operator")
        TARGET_LINK_LIBRARIES(${PACKAGE_RUNNER} tsl::ordered_map paho-mqttpp3)
        TARGET_PRECOMPILE_HEADERS(${PACKAGE_RUNNER} REUSE_FROM ${PROJECT_NAME})
        SET_TARGET_PROPERTIES(${PACKAGE_RUNNER} PROPERTIES
                ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/docs/build"
                LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/docs/build"
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/docs/build"
                OUTPUT_NAME ${PACKAGE_RUNNER}
                SUFFIX ".out")
        MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
    ENDFOREACH ()
    MATH(EXPR COUNTER "${COUNTER}+${TOTAL}")
    SET(TOTAL ${COUNTER} PARENT_SCOPE)
ENDFUNCTION(MAKE_CODE_RUNNERS)
MAKE_CODE_RUNNERS(runners main_runner)
MESSAGE(STATUS "Total runners produced: ${.g}${TOTAL}${..}")
#####################################################################
MESSAGE(CHECK_START "${.y}CMaking AsciiDoc Documentation${..}")
FILE(COPY ${CMAKE_SOURCE_DIR}/docs DESTINATION ${CMAKE_BINARY_DIR})
FILE(COPY ${CMAKE_SOURCE_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})
SET(DOCS_SRC ${CMAKE_BINARY_DIR}/docs/src)
ADD_CUSTOM_TARGET(asciidoc ALL DEPENDS main_runner)
SET(FILES ${DOCS_SRC}/fhatos.adoc)
ADD_CUSTOM_COMMAND(TARGET asciidoc
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND markdown-code-runner ${FILES}
        COMMENT "Evaluating markdown-code-runner: ${FILES}")
ADD_CUSTOM_COMMAND(TARGET asciidoc
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND asciidoctor -D ${CMAKE_BINARY_DIR}/docs -o index.html ${FILES}
        COMMENT "Evaluating asciidoctor: ${FILES}")
FILE(COPY ${CMAKE_BINARY_DIR}/docs/index.html DESTINATION ${CMAKE_SOURCE_DIR}/docs)
MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
UNSET(FILES)
#####################################################################
MESSAGE(CHECK_START "${.y}CMaking Markdown Documentation${..}")
SET(FILES ${DOCS_SRC}/fhatos.md)
ADD_CUSTOM_TARGET(markdown ALL DEPENDS main_runner)
ADD_CUSTOM_COMMAND(TARGET markdown
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND markdown-code-runner ${FILES}
        COMMENT "Evaluating markdown-code-runner: ${R_DONE}")
MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
###
MESSAGE(CHECK_START "${.y}CMaking Doxygen Documentation${..}")
FILE(COPY ${CMAKE_SOURCE_DIR}/docs DESTINATION ${CMAKE_BINARY_DIR})
MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
# ENDIF ()