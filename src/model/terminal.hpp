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
#include <structure/stype/heap.hpp>

namespace fhatos {
  class Terminal final : public Heap {
  protected:
    explicit Terminal(const Pattern &id) : Heap(id) {
    }

  public:
    static ptr<Terminal> singleton(const Pattern &id) {
      static auto terminal_p = ptr<Terminal>(new Terminal(id));
      return terminal_p;
    }

    void setup() override {
      Heap::setup();
      Heap::write(p_p(this->pattern()->resolve("./:owner")), vri("none"), true);
    }

    void write(const fURI_p &furi, const Obj_p &obj, const bool retain) override {
      if (furi->equals(this->pattern()->resolve("./:owner"))) {
        for (const auto &x: *this->subscriptions_) {
          this->recv_unsubscribe(id_p(x->source), p_p(x->pattern));
        }
        router()->route_subscription(subscription_p(ID(this->pattern_->retract_pattern()), Pattern(obj->uri_value()),
                                                    Insts::to_bcode([](const Message_p &message) {
                                                      printer<>()->print(message->payload->str_value().c_str());
                                                    })));
      }
      Heap::write(furi, obj, retain);
    }

    /* ReadRawResult read_raw_pairs(const fURI_p &furi) override {
       if (furi->matches(*this->pattern())) {
         return {
           {
             id_p(this->pattern()->retract_pattern()),
             str(to_string(Terminal::readChar()))
           }};
       }
       return {};
     } */

    static int readChar() {
#ifdef NATIVE
      return getchar();
#else
      return (Serial.available() > 0) ? Serial.read() : EOF;
#endif
    }
  };
} // namespace fhatos
#endif
