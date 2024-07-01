/*******************************************************************************
 FhatOS: A Distributed Operating System
 Copyright (c) 2024 PhaseShift Studio, LLC

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef fhatos_f_serial_hpp
#define fhatos_f_serial_hpp

#include <fhatos.hpp>
//
#include <process/actor/actor.hpp>
#include <process/router/message.hpp>

namespace fhatos {

template <typename PROCESS = Fiber, typename ROUTER = Router>
class fSerial : public Actor<PROCESS, ROUTER> {

protected:
 explicit fSerial(const ID &id = Router::mintID("serial"))
      : Actor<PROCESS, ROUTER>(id) {}

public:
  static void println(const char *text) {
    ROUTER::singleton()->publish(
        Message{.source = "anonymous",
                .target = fSerial::singleton()->id(),
                .payload = {.type = OType::STR,
                            .data = (fbyte *)(String(text) + "\n").c_str(),
                            .length = strlen(text) + 1},
                .retain = false});
  }
  static fSerial *singleton() {
    static fSerial serial = fSerial();
    return &serial;
  }
  virtual void setup() override {
    PROCESS::setup();
    this->subscribe(
        this->id(),
        [](const auto &message) {
          Serial.print(message.payload->toString().c_str());
        },
        QoS::_1);
  }
};
} // namespace fhatos

#endif