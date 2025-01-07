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

#include  "../../src/fhatos.hpp"
#include "../../src/language/obj.hpp"

namespace fhatos {
  using std::make_pair;

  class ObjHelper {
    ObjHelper() = delete;

  public:
    static bool check_coefficients(const IntCoefficient a, const IntCoefficient b, bool throw_on_error = true) {
      if(a.first < b.first || a.second > b.second) {
        if(throw_on_error) {
          throw fError("lhs coefficient not within rhs coefficient: {%i,%i} <> {%i,%i}", a.first, a.second, b.first,
                       b.second);
        }
        return false;
      }
      return true;
    }
  };

  class InstBuilder {
    explicit InstBuilder(const TypeO_p &type) : type_(type), seed_(nullptr) {
    }

  protected:
    ID_p type_;
    InstArgs args_{Obj::to_inst_args()};
    InstF_p function_supplier_ = nullptr;
    Obj_p seed_;
    string doc_{};

  public:
    static InstBuilder *build(const ID &type_id = *INST_FURI) {
      return new InstBuilder(id_p(*ROUTER_RESOLVE(fURI(type_id))));
    }

    InstBuilder *type_args(const Obj_p &arg0, const Obj_p &arg1 = nullptr, const Obj_p &arg2 = nullptr,
                           const Obj_p &arg3 = nullptr, const Obj_p &arg4 = nullptr) {
      this->args_->rec_set("_0", arg0);
      if(arg1)
        this->args_->rec_set("_1", arg1);
      if(arg2)
        this->args_->rec_set("_2", arg2);
      if(arg3)
        this->args_->rec_set("_3", arg3);
      if(arg4)
        this->args_->rec_set("_4", arg4);
      return this;
    }

    InstBuilder *domain_range(const ID_p &domain, const ID_p &range = nullptr) {
      return this->domain_range(domain, {1, 1}, range, {1, 1});
    }

    InstBuilder *domain_range(const ID_p &domain, const IntCoefficient &domain_coefficient, const ID_p &range,
                              const IntCoefficient &range_coefficient) {
      this->type_ = id_p(this->type_->query({
        {FOS_DOMAIN, domain->toString()},
        {FOS_DC_MIN, to_string(domain_coefficient.first)},
        {FOS_DC_MAX, to_string(domain_coefficient.second)},
        {FOS_RANGE, range->toString()},
        {FOS_RC_MIN, to_string(range_coefficient.first)},
        {FOS_RC_MAX, to_string(range_coefficient.second)}}));
      if(!this->seed_)
        this->seed_ = domain_coefficient.second > 1 ? Obj::to_objs() : Obj::to_noobj();
      return this;
    }

    InstBuilder *doc(const string &documentation) {
      this->doc_ = documentation;
      return this;
    }

    InstBuilder *inst_f(const Cpp &inst_f) {
      this->function_supplier_ = make_shared<InstF>(make_shared<Cpp>(inst_f));
      return this;
    }

    InstBuilder *inst_f(const Obj_p &obj) {
      this->function_supplier_ = make_shared<InstF>(obj);
      return this;
    }

    void save(const Obj_p &root = nullptr) const {
      const Inst_p inst = this->create(nullptr, root);
      TYPE_SAVER(id_p(inst->vid_or_tid()->no_query()), inst);
    }

    [[nodiscard]] Inst_p create(const ID_p &value_id = nullptr, const Obj_p &root = nullptr) const {
      if(value_id) {
        if(const Inst_p maybe = ROUTER_READ(value_id); !maybe->is_noobj())
          return maybe;
      }
      const Inst_p inst = Inst::create(make_tuple(
                                         this->args_,
                                         this->function_supplier_,
                                         this->seed_ ? this->seed_ : Obj::to_noobj()),
                                       OType::INST, this->type_,
                                       root ? id_p(root->vid()->extend(*value_id)) : value_id);
      // if(!this->doc_.empty())
      // inst->doc_write(this->doc_);
      delete this;
      return inst;
    }
  };
} // namespace fhatos
#endif
