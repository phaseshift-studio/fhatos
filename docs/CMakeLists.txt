PROJECT(fhatos-docs)
OPTION(BUILD_DOCS "Build documentation" ON) # ON/OFF
IF (TASKSCH_BUILD_WITH_DOCS)
    ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/docs)
ENDIF ()
IF (BUILD_DOCS)
    MESSAGE("${.r}=====> PROCESSING DOCUMENTATION${..}")
    MESSAGE(CHECK_START "${.y}CMaking Doxygen Documentation${..}")
    FIND_PACKAGE(Doxygen)
    IF (DOXYGEN_FOUND)
        MESSAGE(STATUS "Doxygen found: ${DOXYGEN_EXECUTABLE} -- ${DOXYGEN_VERSION}")
        # Set Doxygen input and output files.
        SET(DOXYGEN_INPUT_DIR ${CMAKE_SOURCE_DIR}/docs)
        SET(DOXYGEN_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/docs/doxygen)
        SET(DOXYGEN_INDEX_FILE ${CMAKE_SOURCE_DIR}/docs/xml/index.xml)
        SET(DOXYFILE_IN ${CMAKE_SOURCE_DIR}/docs/Doxyfile.in)
        SET(DOXYFILE_OUT ${CMAKE_SOURCE_DIR}/docs/Doxyfile)
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
        ADD_CUSTOM_TARGET(docs ALL DEPENDS ${DOXYGEN_INDEX_FILE})
    ENDIF ()
    MESSAGE(CHECK_PASS "[${.g}COMPLETE${..}]")
ENDIF ()