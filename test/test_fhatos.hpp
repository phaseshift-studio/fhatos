#ifndef fhatos_kernel__test_fhatos_hpp
#define fhatos_kernel__test_fhatos_hpp

#include <unity.h>
//
#include <fhatos.hpp>
#include <kernel/util/ansi.hpp>

#define UNITY_BEGIN()                                                          \
  Serial.println(fhatos::kernel::ANSI_ART);                                    \
  UnityBegin(__FILE__)

#define SETUP_AND_LOOP()                                                       \
  void setup() {                                                               \
    Serial.begin(FOS_SERIAL_BAUDRATE);                                         \
    delay(2000);                                                               \
    RUN_UNITY_TESTS();                                                         \
  }                                                                            \
  void loop() {}

#define RUN_TESTS(x)                                                           \
  void RUN_UNITY_TESTS() {                                                     \
    UNITY_BEGIN();                                                             \
    x;                                                                         \
    UNITY_END();                                                               \
  }

#endif