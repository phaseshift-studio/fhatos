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
#ifndef fhatos_redirect_hpp
#define fhatos_redirect_hpp

#include "fhatos.hpp"
#include "language/obj.hpp"
#include FOS_PROCESS(coroutine.hpp)

namespace fhatos {
  class Redirect : public Coroutine {
  public:
    Pair<Pattern_p, Pattern_p> read_mapping_;
    Pair<Pattern_p, Pattern_p> write_mapping_;

    explicit Redirect(const ID &id,
                      const Pair<Pattern_p, Pattern_p> &read_mapping,
                      const Pair<Pattern_p, Pattern_p> &write_mapping) : Coroutine(id), read_mapping_(read_mapping),
                                                                         write_mapping_(write_mapping) {
    }

  public:
    static ptr<Redirect> create(const ID &id, const Pair<Pattern_p, Pattern_p> &read_mapping,
                                const Pair<Pattern_p, Pattern_p> &write_mapping) {
      auto redirect_p = ptr<Redirect>(new Redirect(id, read_mapping, write_mapping));
      return redirect_p;
    }

    void setup() override {
      Coroutine::setup();
      //   //remote/soc/gpio/#    /
     // this->write(id_p(this->pattern()->resolve("./0")),
     //             Obj::to_lst({vri(this->read_mapping_.first), vri(this->read_mapping_.second)}),RETAIN_MESSAGE);
     // this->write(id_p(this->pattern()->resolve("./1")),
     //           Obj::to_lst({vri(this->write_mapping_.first), vri(this->write_mapping_.second)}),RETAIN_MESSAGE);
      router()->route_subscription(subscription_p(*this->id(),
                                                  *this->read_mapping_.first,
                                                  Insts::to_bcode([this](const Message_p &message) {
                                                    const fURI_p rewrite = furi_p(
                                                      this->read_mapping_.second->path(message->target.path()));
                                                    const Obj_p payload = message->payload;
                                                    router()->write(rewrite, payload, message->retain);
                                                  })));
       router()->route_subscription(subscription_p(*this->id(),
                                                   *this->write_mapping_.first,
                                                   Insts::to_bcode([this](const Message_p &message) {
                                                     const fURI_p rewrite = furi_p(
                                                       this->write_mapping_.second->path(message->target.path()));
                                                     const Obj_p payload = message->payload;
                                                     router()->write(rewrite, payload, message->retain);
                                                   })));
    }
  };
} // namespace fhatos

#endif
