FIND_PACKAGE(PythonInterp REQUIRED)
OPTION(BUILD_DOCS "Build documentation" OFF) # ON/OFF
IF (TASKSCH_BUILD_WITH_DOCS)
    ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/docs)
ENDIF ()
OPTION(BUILD_CODE_RUNNERS "Build code runners" ON) # ON/OFF
IF (BUILD_CODE_RUNNERS)
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
    ####################################
    ########### TEST SUITES ############
    ####################################
    MAKE_CODE_RUNNERS(runners main_runner)
    MESSAGE(STATUS "Total runners produced: ${.g}${TOTAL}${..}")
ENDIF ()
#####################################################################
IF (BUILD_DOCS)
    MESSAGE("${.r}=====> PROCESSING DOCUMENTATION${..}")
    MESSAGE(CHECK_START "${.y}CMaking Doxygen Documentation${..}")
    FILE(COPY ${CMAKE_SOURCE_DIR}/docs DESTINATION ${CMAKE_BINARY_DIR})
    FIND_PACKAGE(Doxygen)
    IF (DOXYGEN_FOUND)
        MESSAGE(STATUS "Doxygen found: ${DOXYGEN_EXECUTABLE} -- ${DOXYGEN_VERSION}")
        # Set Doxygen input and output files.
        SET(DOXYGEN_INPUT_DIR ${CMAKE_SOURCE_DIR}/docs)
        SET(DOXYGEN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/docs/doxygen)
        SET(DOXYGEN_INDEX_FILE ${CMAKE_BINARY_DIR}/docs/xml/index.xml)
        SET(DOXYFILE_IN ${CMAKE_SOURCE_DIR}/docs/Doxyfile.in)
        SET(DOXYFILE_OUT ${CMAKE_BINARY_DIR}/docs/Doxyfile)
        # Generate DoxyFile from the input file.
        CONFIGURE_FILE(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)
        # Create Output directory.
        FILE(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})
        # Command for generating doc from Doxygen config file.
        ADD_CUSTOM_COMMAND(OUTPUT ${DOXYGEN_INDEX_FILE}
                COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
                MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
                COMMENT "Generating Doxygen Documentation"
                VERBATIM)
        # Create CMake Target for generating doc.
        ADD_CUSTOM_TARGET(docs ALL DEPENDS ${DOXYGEN_INDEX_FILE} ${CMAKE_BINARY_DIR}/docs/build/main_runner.out)
        ADD_CUSTOM_COMMAND(TARGET docs
                POST_BUILD
                DEPENDS
                ${CMAKE_SOURCE_DIR}/docs/src/fhatos.md
                ${CMAKE_BINARY_DIR}/docs/build/main_runner.out
                execute_process(
                COMMAND markdown-code-runner ${CMAKE_SOURCE_DIR}/docs/src/fhatos.md
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                COMMENT "Evaluating markdown-code-runner: ${R_DONE}")
        )
    ENDIF ()
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
ENDIF ()

