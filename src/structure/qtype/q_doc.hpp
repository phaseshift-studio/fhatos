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
#ifndef fhatos_q_doc_hpp
#define fhatos_q_doc_hpp

#include "../q_proc.hpp"
#include "../../fhatos.hpp"

namespace fhatos {
  class QDoc final : public QProc {
  protected:
    unique_ptr<Map<fURI, Obj_p>> doc_data_ = make_unique<Map<fURI, Obj_p>>();

  public:
    explicit QDoc(const ID_p &value_id = nullptr) : QProc(id_p("/fos/q/q_doc"), value_id) {
      this->Obj::rec_set("pattern", vri("doc"));
    }

    static ptr<QDoc> create(const ID &value_id) {
      return make_shared<QDoc>(id_p(value_id));
    }

    void write(const QProc::POSITION pos, const fURI &furi, const Obj_p &obj, const bool retain) override {
      if(retain && POSITION::PRE == pos) {
        this->doc_data_->insert_or_assign(furi.no_query(), obj);
        LOG_WRITE(DEBUG, this,L("!ypre-wrote!! !b{}!! -> {}\n", furi.no_query().toString(), obj->toString()));
      } else if(retain && POSITION::Q_LESS == pos && obj->is_noobj()) {
        this->doc_data_->erase(furi.no_query());
      }
    }

    Obj_p read(const QProc::POSITION pos, const fURI &furi, const Obj_p &post_read) const override {
      if(POSITION::POST == pos)
        return Obj::to_noobj();
      Obj_p return_obj = Obj::to_noobj();
      const fURI furi_no_query = furi.no_query();
      if(furi_no_query.is_pattern()) {
        return_obj = Obj::to_rec();
        for(const auto &[k,v]: *this->doc_data_) {
          if(furi_no_query.matches(k))
            return_obj->rec_set(vri(k), v);
        }
      } else
        return_obj = this->doc_data_->count(furi_no_query) ? this->doc_data_->at(furi_no_query) : Obj::to_noobj();
      LOG_WRITE(DEBUG, this,L("!ypre-read!! !b{}!! -> {}\n", furi_no_query.toString(), return_obj->toString()));
      return return_obj;
    }

    [[nodiscard]] ON_RESULT is_pre_read() const override {
      return ON_RESULT::ONLY_Q;
    }

    [[nodiscard]] ON_RESULT is_pre_write() const override {
      return ON_RESULT::ONLY_Q;
    }

    [[nodiscard]] virtual ON_RESULT is_q_less_write() const {
      return ON_RESULT::IGNORE_Q;
    }
  };
}
#endif
