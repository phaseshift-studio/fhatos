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
#ifndef fhatos_q_default_hpp
#define fhatos_q_default_hpp

#include "../../../fhatos.hpp"
#include "../../../furi.hpp"
#include "../../../structure/q_proc.hpp"

#define Q_DEFAULT_TID FOS_URI "/q/default"

namespace fhatos {
  class QDefault final : public QProc {
  public:
    /*
    /sys/# -> /shared/comp
    */

    ptr<std::map<fURI, Obj_p>> read_map = std::make_shared<std::map<fURI, Obj_p>>();

  public:
    explicit QDefault(const ID_p &value_id = nullptr) : QProc(id_p(Q_DEFAULT_TID), value_id) {
      this->Obj::rec_set("pattern", vri("default"));
    }

    void loop() const override {
      // do nothing
    }

    static ptr<QDefault> create(const ID &value_id) {
      return make_shared<QDefault>(value_id.empty() ? nullptr : id_p(value_id));
    }

    void write(const QProc::POSITION pos, const fURI &furi, const Obj_p &obj, const bool retain) override {
      if(POSITION::PRE == pos) {
        // mirror writes
        this->read_map->insert_or_assign(furi.no_query(), obj);
      }
    }

    Obj_p read(const QProc::POSITION pos, const fURI &furi, const Obj_p &post_read) const override {
      const fURI furi_no_query = furi.no_query();
      const Obj_p results = Objs::to_objs();
      /// pre-read
      if(POSITION::Q_LESS == pos && post_read->none_one_all()->is_noobj()) {
        for(const auto &[u, o]: *this->read_map) {
          if(furi_no_query.bimatches(u)) {
            results->add_obj(o->apply(vri(furi)));
          }
        }
      } else if(POSITION::PRE == pos) {
        for(const auto &[u, o]: *this->read_map) {
          if(furi_no_query.bimatches(u)) {
            results->add_obj(o);
          }
        }
      }
      return results;
    }

    [[nodiscard]] ON_RESULT is_pre_read() const override { return ON_RESULT::ONLY_Q; }

    [[nodiscard]] ON_RESULT is_pre_write() const override { return ON_RESULT::ONLY_Q; }

    [[nodiscard]] ON_RESULT is_q_less_read() const override { return ON_RESULT::INCLUDE_Q; }
  };
} // namespace fhatos
#endif
