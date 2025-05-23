FIND_PACKAGE(Python3)
OPTION(BUILD_DOCS "Build documentation" ON) # ON/OFF
SET(CMAKE_MACOSX_RPATH 1)
IF(TASKSCH_BUILD_WITH_DOCS)
    ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/docs)
ENDIF()
IF(BUILD_DOCS)
    MESSAGE("\n${.g}=====> ${.r}P${.y}R${.m}O${.y}C${.b}E${.m}S${.g}S${.c}I${.y}N${.r}G ${.m}D${.c}O${.g}C${.r}S${..}\n")
    MESSAGE(STATUS "target linked libraries: ${TARGET_LIBRARIES}")
    SET(TOTAL 0)
    FUNCTION(MAKE_CODE_RUNNERS PACKAGE PACKAGE_RUNNERS)
        SET(COUNTER 0)
        MESSAGE(STATUS "${.m}Processing " ${PACKAGE} " with " ${PACKAGE_RUNNERS}${..})
        FOREACH(PACKAGE_RUNNER ${PACKAGE_RUNNERS})
            MATH(EXPR COUNTER "${COUNTER}+1")
            MESSAGE(CHECK_START "${.y}CMaking ${PACKAGE_RUNNER}${..}")
            ADD_DEFINITIONS(-DNATIVE)
            ADD_EXECUTABLE(${PACKAGE_RUNNER} ${CMAKE_SOURCE_DIR}/docs/src/${PACKAGE}/${PACKAGE_RUNNER}.cpp)
            IF(USE_PAHO_MQTT_HACK)
                MESSAGE(CHECK_START "${.r}Performing Paho-MQTT Build Hack ${.g}[${.y}TEST SUITE${.g}]${..}")
                TARGET_LINK_LIBRARIES(${PACKAGE_RUNNER} PUBLIC ${TARGET_LIBRARIES})
                MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
            ELSE()
                TARGET_LINK_LIBRARIES(${PACKAGE_RUNNER} ${TARGET_LIBRARIES})
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
    FILE(COPY ${CMAKE_SOURCE_DIR}/docs/src/runner/fhatos-doc-code.py DESTINATION ${CMAKE_BINARY_DIR}/docs/build)
    FILE(COPY ${CMAKE_SOURCE_DIR}/scripts/colors.py DESTINATION ${CMAKE_BINARY_DIR}/docs/build)
    FILE(COPY ${CMAKE_SOURCE_DIR}/docs/fhatos_index.adoc DESTINATION ${CMAKE_BINARY_DIR}/docs)
    FILE(COPY ${CMAKE_SOURCE_DIR}/docs/fhatos_intro.adoc DESTINATION ${CMAKE_BINARY_DIR}/docs)
    FILE(COPY ${CMAKE_SOURCE_DIR}/docs/fhatos_mmadt.adoc DESTINATION ${CMAKE_BINARY_DIR}/docs)
    FILE(COPY ${CMAKE_SOURCE_DIR}/docs/fhatos_router.adoc DESTINATION ${CMAKE_BINARY_DIR}/docs)
    FILE(COPY ${CMAKE_SOURCE_DIR}/docs/data/boot/doc_boot_config.obj DESTINATION ${CMAKE_BINARY_DIR}/docs/build/data/boot)
    FILE(COPY ${CMAKE_SOURCE_DIR}/docs/images DESTINATION ${CMAKE_BINARY_DIR}/docs)
    FILE(COPY ${CMAKE_SOURCE_DIR}/docs/css DESTINATION ${CMAKE_BINARY_DIR}/docs)
    FILE(COPY ${CMAKE_SOURCE_DIR}/docs/highlight DESTINATION ${CMAKE_BINARY_DIR}/docs)
    FILE(REMOVE_RECURSE ${CMAKE_SOURCE_DIR}/docs/images/diagrams)
    ADD_CUSTOM_TARGET(asciidoc ALL DEPENDS main_runner boot_runner)
    SET(INDEX ${CMAKE_BINARY_DIR}/docs/fhatos_index.adoc)
    SET(FILES ${CMAKE_BINARY_DIR}/docs/fhatos_intro.adoc)
    SET(FILES ${CMAKE_BINARY_DIR}/docs/fhatos_mmadt.adoc)
    SET(FILES ${CMAKE_BINARY_DIR}/docs/fhatos_router.adoc)
    SET(SINGLE ${CMAKE_BINARY_DIR}/docs/fhatos.adoc)
    ADD_CUSTOM_COMMAND(TARGET asciidoc
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/docs
            COMMAND asciidoctor-reducer -o ${SINGLE} ${INDEX}
            COMMENT "Evaluating asciidoctor-reducer: ${INDEX} of ${FILES}")
    ADD_CUSTOM_COMMAND(TARGET asciidoc
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/docs/build
            COMMAND python fhatos-doc-code.py --verbose ${SINGLE}
            COMMENT "Evaluating fhatos-doc-code.py: ${SINGLE}")
    ADD_CUSTOM_COMMAND(TARGET asciidoc
            POST_BUILD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/docs
            COMMAND asciidoctor -v -r asciidoctor-tabs -r asciidoctor-diagram -o index.html ${SINGLE}
            COMMENT "Evaluating asciidoctor: ${SINGLE}")
    UNSET(SINGLE)
    UNSET(FILES)
    UNSET(INDEX)
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
    #####################################################################
ENDIF()