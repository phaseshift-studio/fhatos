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
#include "structure/pubsub.hpp"
#include "structure/router.hpp"

namespace fhatos {
  class Redirect {


  public:
    static ptr<Obj> create(const ID &id, const Pair<Pattern_p, Pattern_p> &read_mapping,
                           const Pair<Pattern_p, Pattern_p> &write_mapping) {
      router()->route_subscription(
          subscription_p(id, *read_mapping.first, Subscription::to_bcode([read_mapping](const Message_p &message) {
                           const fURI_p rewrite = furi_p(read_mapping.second->segment(message->target.path()));
                           const Obj_p payload = message->payload;
                           router()->pull_exec(id_p(*rewrite), payload);
                         })));
      router()->route_subscription(
          subscription_p(id, *write_mapping.first, Subscription::to_bcode([write_mapping](const Message_p &message) {
                           const fURI_p rewrite = furi_p(write_mapping.second->segment(message->target.path()));
                           const Obj_p payload = message->payload;
                           router()->pull_exec(id_p(*rewrite), payload);
                         })));
      return noobj();
    }
  };
} // namespace fhatos

#endif
