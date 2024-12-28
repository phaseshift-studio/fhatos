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

#ifndef fhatos_computed_hpp
#define fhatos_computed_hpp

#include <fhatos.hpp>
#include <furi.hpp>
#include <language/obj.hpp>
#include <structure/structure.hpp>

namespace fhatos {
  class Computed : public Structure {
  protected:
    //<query, function<query, <id,result>>
    shared_ptr<Map<fURI_p, Function<fURI_p, IdObjPairs>, furi_p_less>> read_functions_;
    shared_ptr<Map<fURI_p, BiFunction<fURI_p, Obj_p, IdObjPairs>, furi_p_less>> write_functions_;

    explicit Computed(
      const Pattern &pattern,
      const ID &vid,
      const Map<fURI_p, Function<fURI_p, IdObjPairs>, furi_p_less> &read_map = {},
      const Map<fURI_p, BiFunction<fURI_p, Obj_p, IdObjPairs>, furi_p_less> &write_map = {}) : Structure(pattern, vid),
      read_functions_(
        make_unique<Map<fURI_p, Function<fURI_p, List<Pair<ID_p, Obj_p>>>, furi_p_less>>(read_map)),
      write_functions_(
        make_unique<Map<fURI_p, BiFunction<fURI_p, Obj_p, List<Pair<ID_p, Obj_p>>>, furi_p_less>>(
          write_map)) {
    }

    void write_raw_pairs(const ID_p &id, const Obj_p &obj, const bool retain) override {
      if(retain) {
        for(const auto &[furi, func]: *this->write_functions_) {
          if(id->matches(*furi)) {
            scheduler()->feed_local_watchdog();
            func(id, obj);
            scheduler()->feed_local_watchdog();
            LOG_STRUCTURE(DEBUG, this, "!g%s!y=>!g%s!! written\n", id->toString().c_str(), obj->toString().c_str());
          }
        }
      }
      this->distribute_to_subscribers(Message::create(*id, obj, retain));
    }

    IdObjPairs read_raw_pairs(const fURI_p &furi) override {
      auto list = IdObjPairs();
      for(const auto &[furi2, func]: *this->read_functions_) {
        if(furi->bimatches(*furi2)) {
          scheduler()->feed_local_watchdog();
          const IdObjPairs list2 = func(furi);
          list.insert(list.end(), list2.begin(), list2.end());
          scheduler()->feed_local_watchdog();
        }
      }
      return list;
    }
  };
} // namespace fhatos

#endif
