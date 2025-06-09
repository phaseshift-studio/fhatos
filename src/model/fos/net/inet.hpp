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
#ifndef fhatos_inet_hpp
#define fhatos_inet_hpp
#include "../../../fhatos.hpp"
#include "../../../furi.hpp"
#include "../../../lang/mmadt/mmadt.hpp"
#include "../../../util/obj_helper.hpp"
#include "../sys/typer/typer.hpp"

#define INET_TID FOS_URI "/net/inet"

namespace fhatos {
  using namespace mmadt;
  class Inet {
  public:
    static Rec_p ip_addresses();
    static ID mdns();
    static void register_module() {
      REGISTERED_MODULES->insert_or_assign(
          ID(INET_TID),
          InstBuilder::build(Typer::singleton()->vid->add_component(INET_TID))
              ->domain_range(OBJ_FURI, {0, 1}, OBJ_FURI, {0, 0})
              ->inst_f([](const Obj_p &, const InstArgs &) {
                return Obj::to_rec(
                    {{vri(INET_TID), Obj::to_rec({{vri("ip4"), InstBuilder::build(ID(INET_TID "/ip4"))
                                                                   ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {1, 1})
                                                                   ->inst_f([](const Obj_p &, const InstArgs &args) {
                                                                     return Inet::ip_addresses();
                                                                   })
                                                                   ->create()},
                                                  {vri("mdns"), InstBuilder::build(ID(INET_TID "/mdns"))
                                                                    ->domain_range(OBJ_FURI, {0, 1}, URI_FURI, {1, 1})
                                                                    ->inst_f([](const Obj_p &, const InstArgs &args) {
                                                                      return vri(Inet::mdns());
                                                                    })
                                                                    ->create()}})}});
              })
              ->create());
    }
  };
} // namespace fhatos

#endif
