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

#pragma once
#ifndef fhatos_terminal_hpp
#define fhatos_terminal_hpp

#include <fhatos.hpp>
#include <iostream>
#include <process/actor/actor.hpp>
#include FOS_PROCESS(coroutine.hpp)
#include <structure/stype/id_structure.hpp>

namespace fhatos {
  class Terminal final : public Actor<Coroutine, IDStructure> {
  protected:
    explicit Terminal(const ID &id = ID("/io/terminal/")) : Actor(id) {}

  public:
    static ptr<Terminal> singleton(const ID &id = "/io/terminal/") {
      static auto terminal_p = ptr<Terminal>(new Terminal(id));
      return terminal_p;
    }

    void setup() override {
      Actor::setup();
      this->subscribe(*this->id_, [this](const Message_p &message) {
        if (message->retain && message->payload->is_noobj())
          this->stop();
        else
          printer<>()->print(message->payload->str_value().c_str());
      });
    }

    static int readChar() {
#ifdef NATIVE
      return getchar();
#else
      return (Serial.available() > 0) ? Serial.read() : EOF;
#endif
    }

    static void out(const ID_p &, const char *format, ...) {
      char buffer[FOS_DEFAULT_BUFFER_SIZE];
      va_list arg;
      va_start(arg, format);
      const int length = vsnprintf(buffer, FOS_DEFAULT_BUFFER_SIZE, format, arg);
      buffer[length] = '\0';
      va_end(arg);
      router()->route_message(share(Message{//
                                            .target = *Terminal::singleton()->id(), //
                                            .payload = Obj::to_str(buffer),
                                            .retain = TRANSIENT_MESSAGE}));
    }
  };
} // namespace fhatos
#endif
