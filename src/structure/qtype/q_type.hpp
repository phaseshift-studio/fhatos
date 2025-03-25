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
#ifndef fhatos_q_type_hpp
#define fhatos_q_type_hpp

#include "../q_proc.hpp"
#include "../../fhatos.hpp"
#include "../../furi.hpp"
#include "../../lang/mmadt/compiler.hpp"

namespace fhatos {
  class QType final : public QProc {
  public:
    explicit QType(const ID_p &value_id = nullptr) : QProc(REC_FURI, value_id) {
      this->Obj::rec_set("pattern", vri("#"));
    }

    static ptr<QType> create(const ID_p &value_id = nullptr) {
      return make_shared<QType>(value_id);
    }

    void write(const QProc::POSITION pos, const fURI &furi, const Obj_p &obj, const bool retain) override {
      if(string(furi.query()).find("=") == string::npos)
        Compiler(true, true).type_check(obj, furi.query());
    }

    [[nodiscard]] Obj_p read(const QProc::POSITION pos, const fURI &furi, const Obj_p &post_read) const override {
      if(!post_read->is_noobj() && string(furi.query()).find("=") == string::npos)
        Compiler(true, true).type_check(post_read->is_objs() && post_read->objs_value()->size() == 1
                                          ? post_read->objs_value()->front()
                                          : post_read, furi.query());
      return post_read;
    }

    [[nodiscard]] ON_RESULT is_post_read() const override {
      return ON_RESULT::IGNORE_Q;
    }

    [[nodiscard]] ON_RESULT is_pre_write() const override {
      return ON_RESULT::IGNORE_Q;
    }
  };
}
#endif
