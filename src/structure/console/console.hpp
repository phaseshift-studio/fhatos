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

#ifndef fhatos_console_hpp
#define fhatos_console_hpp

#include <fhatos.hpp>
#include <iostream>
#include <language/fluent.hpp>
#include <language/parser.hpp>
#include <structure/furi.hpp>
#include <util/ansi.hpp>
#include <util/string_helper.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_MQTT(mqtt_router.hpp)

namespace fhatos {
  class Console final : public Thread {
  public:
    explicit Console(const ID &id = ID("thread/console")) : Thread(id) {}

    void setup() override { Thread::setup(); }

    void loop() override {
      Thread::loop();
      string line;
      while (true) {
        this->printPrompt();
        line.clear();
        std::getline(std::cin, line);
        StringHelper::trim(line);
        /////
        if (line.empty()) {
          // do nothing
        } else if (line[0] == ':') {
          if (line == ":quit") {
            this->stop();
            return;
          } else if (strstr(line.c_str(), ":log")) {
            if (line.length() < 6) {
              this->printResult(Obj::to_str(LOG_TYPES.toChars((LOG_TYPE) GLOBAL_OPTIONS->LOGGING)));
            } else {
              string level = line.substr(5);
              GLOBAL_OPTIONS->LOGGING = LOG_TYPES.toEnum(level.c_str());
            }
          } else if (strstr(line.c_str(), ":router")) {
            if (line.length() < 9) {
              this->printResult(Obj::to_str(GLOBAL_OPTIONS->router<Router>()->toString()));
            } else {
              string router = line.substr(8);
              if (router == "LocalRouter")
                GLOBAL_OPTIONS->ROUTING = LocalRouter::singleton();
              else if (router == "MqttRouter")
                GLOBAL_OPTIONS->ROUTING = MqttRouter::singleton();
              else
                this->printException(fError("Invalid logger (LocalRouter,MqttRouter): %s\n", router.c_str()));
            }
          }
        } else {
          try {
            const Option<Obj_p> obj = Parser::singleton()->tryParseObj(line);
            if (obj.has_value()) {
              if (obj.value()->isBytecode())
                this->printResults(Fluent(obj.value()));
              else
                this->printResult(obj.value());
            } else {
              this->printException(fError("Unable to parse input: %s\n", line.c_str()));
            }
          } catch (const std::exception &e) {
            this->printException(e);
          }
        }
      }
    }
    void stop() override { Thread::stop(); }
    ///// printers
    void printException(const std::exception &ex) const {
      GLOBAL_OPTIONS->printer()->printf("!r[ERROR]!! %s", ex.what());
    }
    void printPrompt() const { GLOBAL_OPTIONS->printer()->print("!mfhatos!!> "); }
    void printResults(const Fluent &fluent) const {
      fluent.forEach<Obj>([this](const Obj_p obj) { this->printResult(obj); });
    }
    void printResult(const Obj_p &obj) const {
      GLOBAL_OPTIONS->printer()->printf("!g==>!!%s\n", obj->toString().c_str());
    }
  };
} // namespace fhatos

#endif
