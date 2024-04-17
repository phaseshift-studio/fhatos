#include <fhatos.hpp>
#include <kernel/util/ansi.hpp>

void setup() {
  Serial.begin(9600);
  Serial.println(fhatos::kernel::ANSI_ART);
}

void loop() {}