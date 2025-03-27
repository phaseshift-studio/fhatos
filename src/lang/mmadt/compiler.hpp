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
#ifndef fhatos_compiler_hpp
#define fhatos_compiler_hpp

#include "../../fhatos.hpp"
#include "../../furi.hpp"

template<typename T>
class Coefficient;
using std::tuple;
using std::vector;
using std::make_tuple;


namespace fhatos {
  class Obj;
  using IntCoefficient = std::pair<int, int>;
  using InstArgs = std::shared_ptr<const Obj>;
  using Obj_p = std::shared_ptr<const Obj>;
  using Obj_wp = std::weak_ptr<Obj>;
  using Inst_p = std::shared_ptr<const Obj>;
  using ID_p = std::shared_ptr<ID>;


  class Compiler {
  public:
    static bool boot_loading;

    enum class Algorithm { SINGLE_PASS, INST_RESOLUTION, OPTIMIZE };

    using DerivationTree = vector<tuple<ID_p, ID_p, Obj_p>>;
    ///////////////////////////////////////////////////////
    bool throw_on_miss;
    ptr<DerivationTree> dt;

    explicit Compiler(bool throw_on_miss = true, bool with_derivation = false);

    //Obj_p compile(const Obj_p& starts, const BCode_p& bcode, const Algorithm compilation_algo);
    //Obj_p rewrite(const Obj_p& starts, const BCode_p& bcode, const vector<Inst_p>& rewrite_rules);
    //void explain(const Obj_p& starts, const BCode_p& bcode, const string* output);

    void print_derivation_tree(string *derivation_string) const;

    [[nodiscard]] const Compiler *reset() const {
      if(dt)
        dt->clear();
      return this;
    }

    const Compiler *reset(const bool throw_on_miss, const bool with_derivation) {
      if(dt)
        dt->clear();
      if(!with_derivation)
        this->dt = nullptr;
      this->throw_on_miss = throw_on_miss;
      return this;
    }

    static bool in_block_list(const string &op);

    [[nodiscard]] Inst_p resolve_inst(const Obj_p &lhs, const Inst_p &inst) const;

    template<typename COEF = IntCoefficient>
    bool coefficient_check(const COEF &lhs, const COEF &rhs) const;

    bool type_check(const Obj *value_obj, const ID &type_id) const;

    [[nodiscard]] bool type_check(const Obj_p &value_obj, const ID &type_id) const {
      return this->type_check(value_obj.get(), type_id);
    }

    [[nodiscard]] Obj_p super_type(const Obj_p &value_obj) const;

  private:
    [[nodiscard]] Inst_p convert_to_inst(const Obj_p &lhs, const Inst_p &stub_inst, const Obj_p &inst_obj) const;

    [[nodiscard]] Inst_p merge_inst(const Obj_p &lhs, const Inst_p &inst_provided, const Inst_p &inst_resolved) const;
  };
}
#endif
