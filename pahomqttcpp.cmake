#
# Builds the paho.mqtt.cpp library
#
# Outputs the following target:
#   paho-mqttpp3
#
include(ExternalProject)

set(PAHOMQTTCPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/paho.mqtt.cpp)
set(PAHOMQTTCPP_TARGET_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/paho.mqtt.cpp)
set(PAHOMQTTCPP_STATIC_LIB ${PAHOMQTTCPP_TARGET_DIR}/lib/libpaho-mqttpp3.a)

file(MAKE_DIRECTORY ${PAHOMQTTCPP_TARGET_DIR}/include)

set(PAHO_BUILD_SHARED FALSE)
if(NOT ${PAHO_BUILD_STATIC})
    set(PAHO_BUILD_SHARED TRUE)
endif()

ExternalProject_Add(
        pahomqttcpp
        DEPENDS pahomqttc
        PREFIX ${PAHOMQTTCPP_TARGET_DIR}
        GIT_REPOSITORY https://github.com/laitingsheng/paho.mqtt.cpp.git
        SOURCE_DIR ${PAHOMQTTCPP_DIR}
        CMAKE_ARGS -DCMAKE_PROJECT_INCLUDE=${CMAKE_CURRENT_BINARY_DIR}/fix_pahomqttcpp.cmake
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PAHOMQTTCPP_TARGET_DIR}
        CMAKE_ARGS -DPAHO_WITH_SSL=${PAHO_BUILD_WITH_SSL}
        CMAKE_ARGS -DPAHO_BUILD_STATIC=${PAHO_BUILD_STATIC}
        CMAKE_ARGS -DPAHO_BUILD_SHARED=${PAHO_BUILD_SHARED}
        BUILD_COMMAND make
        INSTALL_COMMAND make install
        BUILD_BYPRODUCTS ${PAHOMQTTCPP_STATIC_LIB}
)

add_library(paho-mqttpp3 STATIC IMPORTED GLOBAL)

add_dependencies(paho-mqttpp3 pahomqttcpp)

set_target_properties(paho-mqttpp3 PROPERTIES IMPORTED_LOCATION ${PAHOMQTTCPP_STATIC_LIB})
set_target_properties(paho-mqttpp3 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${PAHOMQTTCPP_TARGET_DIR}/include)
