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

#ifndef fhatos_f_log_hpp
#define fhatos_f_log_hpp

#include <fhatos.hpp>
//
#include <structure/furi.hpp>
#include <process/actor/actor.hpp>
#include <process/router/local_router.hpp>
#include <util/ansi.hpp>
#include <util/string_printer.hpp>

namespace fhatos {

template <typename PROCESS = Coroutine, typename ROUTER = Router>
class fLog : public Actor<PROCESS, ROUTER> {
public:
  explicit fLog(const ID &id = Router::mintID("log"))
      : Actor<PROCESS, ROUTER>(id) {};

  void setup() override {
    PROCESS::setup();
    const ID serialID = Router::mintID("log");
    // INFO LOGGING
    this->subscribe(
        this->id().extend("INFO"), [this, serialID](const auto &message) {
          this->publish(
              serialID,
              this->createLogMessage(INFO, message.payload->toString()),
              false);
        });
    // ERROR LOGGING
    this->subscribe(
        this->id().extend("ERROR"), [this, serialID](const auto &message) {
          this->publish(
              serialID,
              this->createLogMessage(INFO, message.payload->toString()),
              false);
        });
  }

protected:
  string createLogMessage(LOG_TYPE type, const string& message) {
    if (message[0] == '\t')
      type = LOG_TYPE::NONE;
    string output;
    StringPrinter stream = StringPrinter(&output);
    auto ansi = Ansi<StringPrinter>(&stream);
    if (type != LOG_TYPE::NONE) {
      if (type == ERROR)
        ansi.print("!r[ERROR]!!  ");
      else if (type == INFO)
        ansi.print("!g[INFO]!!  ");
      else
        ansi.print("!y[DEBUG]!!  ");
    }
    ansi.print(message.c_str());
    return string(output.c_str());
  }
};
} // namespace fhatos

#endif