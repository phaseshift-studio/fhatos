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
#ifndef fhatos_q_proc_hpp
#define fhatos_q_proc_hpp

#include "../../../furi.hpp"
#include "../../../lang/obj.hpp"

namespace fhatos {
  class QProc : public Rec {
  public:
    enum class POSITION {
      PRE,
      POST,
      Q_LESS,
    };

    enum class ON_RESULT {
      NO_Q, // do nothing
      IGNORE_Q, // ignore result from qproc
      INCLUDE_Q, // include result from qproc
      ONLY_Q // only return result from qproc
    };

    explicit QProc(const ID_p &type_id, const ID_p &value_id = nullptr): Obj(make_shared<RecMap<>>(),
                                                                             OType::REC,
                                                                             type_id, value_id) {
    }

    static void register_module(const Obj_p& module) {

    }

    ~QProc() override = default;

    virtual void loop() const {
    }

    [[nodiscard]] fURI q_key() const {
      return this->rec_get("pattern")->uri_value();
    }

    [[nodiscard]] virtual Obj_p read(POSITION position, const fURI &furi, const Obj_p &post_read) const = 0;

    virtual void write(POSITION position, const fURI &furi, const Obj_p &obj, bool retain) = 0;

    [[nodiscard]] virtual ON_RESULT is_pre_read() const {
      return ON_RESULT::NO_Q;
    }

    [[nodiscard]] virtual ON_RESULT is_post_read() const {
      return ON_RESULT::NO_Q;
    }

    [[nodiscard]] virtual ON_RESULT is_pre_write() const {
      return ON_RESULT::NO_Q;
    }

    [[nodiscard]] virtual ON_RESULT is_post_write() const {
      return ON_RESULT::NO_Q;
    }

    [[nodiscard]] virtual ON_RESULT is_q_less_write() const {
      return ON_RESULT::NO_Q;
    }

    [[nodiscard]] virtual ON_RESULT is_q_less_read() const {
      return ON_RESULT::NO_Q;
    }
  };
}

#endif
