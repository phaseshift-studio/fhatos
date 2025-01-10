#####################################################################################
######################### FHATOS TARGET (LIBRARY) ###################################
#####################################################################################
FILE(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src" "*.cpp")
MESSAGE(STATUS "${.y}adding source files${..}: ${.g}${SOURCES}${..}")
FILE(GLOB_RECURSE HEADERS "${CMAKE_SOURCE_DIR}/src" "*.hpp")
MESSAGE(STATUS "${.y}adding header files${..}: ${.g}${HEADERS}${..}")
#####################################################################################
INCLUDE("${PROJECT_SOURCE_DIR}/cmake/fhatos_functions.cmake")
ADD_LIBRARY(${PROJECT_NAME}-lib ${SOURCES} ${HEADERS})
CREATE_TARGET(${PROJECT_NAME}-lib)
SOURCE_GROUP(${PROJECT_NAME}-lib FILES ${SOURCES} ${HEADERS})
#####################################################################################