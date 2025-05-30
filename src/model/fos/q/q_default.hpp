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
#ifndef fhatos_q_sub_hpp
#define fhatos_q_sub_hpp

#include "../../../fhatos.hpp"
#include "../../../structure/pubsub.hpp"
#include "../../../structure/q_proc.hpp"
#include "../../../util/mutex_map.hpp"

namespace fhatos {
  class QDefault final : public QProc {
  protected:
    uptr<MutexMap<fURI,  Obj_p>> data_ = make_unique<MutexMap<fURI,  Obj_p>>();

  public:
    explicit QDefault(const ID_p &value_id = nullptr) :
      QProc(REC_FURI, value_id) {
      this->Obj::rec_set("pattern", vri("default"));
    }

    static ptr<QDefault> create(const ID_p &value_id = nullptr) {
      //TYPE_SAVER("/fos/q/q_sub", Obj::to_rec());
      return make_shared<QDefault>(value_id);
    }

    void write(const QProc::POSITION pos, const fURI &furi, const  Obj_p &obj, const bool retain) override {
      const fURI furi_no_query = furi.no_query();
      this->data_->insert_or_assign(furi_no_query, std::move(obj));
    }

    Obj_p read(const QProc::POSITION pos, const fURI &furi, const Obj_p &post_read) const override {
      const fURI furi_no_query = furi.no_query();
      if(post_read->is_noobj()) {
        for(const auto &[k,v]: *this->data_) {
          if(furi_no_query.bimatches(k)) {
            return v;
          }
        }
      }
      return Obj::to_noobj();
    }

    [[nodiscard]] ON_RESULT is_q_less_read() const override {
      return ON_RESULT::INCLUDE_Q;
    }

    [[nodiscard]] ON_RESULT is_pre_write() const override {
      return ON_RESULT::ONLY_Q;
    }
  };
}
#endif
