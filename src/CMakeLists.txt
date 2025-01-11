#####################################################################################
######################### FHATOS TARGET (LIBRARY) ###################################
#####################################################################################
FILE(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src" "*.cpp")
MESSAGE(STATUS "${.y}adding source files${..}: ${.g}${SOURCES}${..}")
FILE(GLOB_RECURSE HEADERS "${CMAKE_SOURCE_DIR}/src" "*.hpp")
MESSAGE(STATUS "${.y}adding header files${..}: ${.g}${HEADERS}${..}")
#####################################################################################
INCLUDE("${PROJECT_SOURCE_DIR}/cmake/fhatos_functions.cmake")
SET(MAIN_LESS_SOURCES ${SOURCES})
LIST(REMOVE_ITEM MAIN_LESS_SOURCES "${CMAKE_SOURCE_DIR}/src/main.cpp")
ADD_LIBRARY(${PROJECT_NAME}-lib ${MAIN_LESS_SOURCES} ${HEADERS})

CREATE_TARGET(${PROJECT_NAME}-lib)
SOURCE_GROUP(${PROJECT_NAME}-lib FILES ${SOURCES} ${HEADERS})
#####################################################################################