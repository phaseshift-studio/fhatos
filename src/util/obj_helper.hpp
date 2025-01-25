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
#ifndef fhatos_obj_helper_hpp
#define fhatos_obj_helper_hpp

#include  "../fhatos.hpp"
#include "../lang/obj.hpp"

namespace fhatos {
  using std::make_pair;
  using std::vector;

  class ObjHelper {
  public:
    ObjHelper() = delete;

    static bool check_coefficients(const IntCoefficient &a, const IntCoefficient &b, const bool throw_on_error = true);

    static bool check_noobj(const ID_p &type_id);
  };

  class InstBuilder {
    explicit InstBuilder(TypeO_p type);

  protected:
    ID_p type_;
    InstArgs args_;
    InstF_p function_supplier_ = nullptr;
    Obj_p seed_;
    string doc_{};

  public:
    static InstBuilder *build(const ID &type_id = *INST_FURI);
    static InstBuilder *build(const fURI_p &type_id = INST_FURI);
    InstBuilder *inst_args(const Rec_p &args);
    InstBuilder *type_args(const Obj_p &arg0, const Obj_p &arg1 = nullptr, const Obj_p &arg2 = nullptr,
                           const Obj_p &arg3 = nullptr, const Obj_p &arg4 = nullptr);
    InstBuilder *domain_range(const ID_p &domain, const ID_p &range = nullptr);
    InstBuilder *domain_range(const ID_p &domain, const IntCoefficient &domain_coefficient, const ID_p &range,
                              const IntCoefficient &range_coefficient);
    InstBuilder *doc(const string &documentation);
    InstBuilder *inst_f(const Cpp &inst_f);
    InstBuilder *inst_f(const Obj_p &obj);
    void save(const Obj_p &root = nullptr) const;
    [[nodiscard]] Inst_p create(const ID_p &value_id = nullptr, const Obj_p &root = nullptr) const;
  };
} // namespace fhatos
#endif
