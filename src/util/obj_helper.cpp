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
#include "obj_helper.hpp"
#include "../lang/obj.hpp"

namespace fhatos {
  using std::make_pair;
  using std::vector;

  InstBuilder::InstBuilder(TypeO_p type) : type_(std::move(type)), args_(Obj::to_rec()), seed_(nullptr) {}


  InstBuilder *InstBuilder::build(const ID &type_id) {
    return new InstBuilder(id_p(ROUTER_RESOLVE(static_cast<fURI>(type_id))));
  }

  InstBuilder *InstBuilder::build(const fURI_p &type_id) { return new InstBuilder(id_p(type_id)); }


  InstBuilder *InstBuilder::inst_args(const Poly_p &args) {
    this->args_ = args->is_lst() ? Obj::to_inst_args(*args->lst_value()) : args;
    return this;
  }

  InstBuilder *InstBuilder::inst_args(const char *arg1_name, const Obj_p &arg1_obj, const char *arg2_name,
                                      const Obj_p &arg2_obj, const char *arg3_name, const Obj_p &arg3_obj) {
    this->args_ = Obj::to_inst_args();
    this->args_->rec_set(arg1_name, arg1_obj);
    if(arg2_name)
      this->args_->rec_set(arg2_name, arg2_obj);
    if(arg3_name)
      this->args_->rec_set(arg3_name, arg3_obj);
    return this;
  }

  InstBuilder *InstBuilder::inst_args_objs(const Obj_p &arg0, const Obj_p &arg1, const Obj_p &arg2, const Obj_p &arg3,
                                      const Obj_p &arg4) {
    this->args_->rec_set(vri("0"), arg0);
    if(arg1)
      this->args_->rec_set(vri("1"), arg1);
    if(arg2)
      this->args_->rec_set(vri("2"), arg2);
    if(arg3)
      this->args_->rec_set(vri("3"), arg3);
    if(arg4)
      this->args_->rec_set(vri("4"), arg4);
    return this;
  }

  InstBuilder *InstBuilder::domain_range(const ID_p &domain, const ID_p &range) {
    return this->domain_range(domain, {1, 1}, range, {1, 1});
  }

  InstBuilder *InstBuilder::domain_range(const ID_p &domain, const IntCoefficient &domain_coefficient,
                                         const ID_p &range, const IntCoefficient &range_coefficient) {
    this->type_ = id_p(this->type_->query(
        {{FOS_DOMAIN, domain->toString()},
         {FOS_DOM_COEF, to_string(domain_coefficient.first).append(",").append(to_string(domain_coefficient.second))},
         {FOS_RANGE, range->toString()},
         {FOS_RNG_COEF, to_string(range_coefficient.first).append(",").append(to_string(range_coefficient.second))}}));
    if(!this->seed_)
      this->seed_ = domain_coefficient.second > 1 ? Obj::to_objs() : Obj::to_noobj();
    return this;
  }

  InstBuilder *InstBuilder::doc(const string &documentation) {
    this->doc_ = documentation;
    return this;
  }

  InstBuilder *InstBuilder::inst_f(const Cpp &inst_f) {
    this->function_supplier_ = InstF(make_shared<Cpp>(inst_f));
    return this;
  }

  InstBuilder *InstBuilder::inst_f(const Obj_p &obj) {
    this->function_supplier_ = InstF(obj);
    return this;
  }

  void InstBuilder::save(const Obj_p &root) const {
    const Inst_p inst = this->create(nullptr, root);
    TYPER_SAVE_TYPE(inst->vid_or_tid()->no_query(), inst);
  }

  Inst_p InstBuilder::create(const ID_p &value_id, const Obj_p &root) const {
    if(value_id) {
      if(const Inst_p maybe = ROUTER_READ(*value_id); !maybe->is_noobj())
        return maybe;
    }
    const Inst_p inst =
        Inst::create(InstValue(this->args_, this->function_supplier_, this->seed_ ? this->seed_ : Obj::to_noobj()),
                     OType::INST, this->type_, root ? id_p(root->vid->extend(*value_id)) : value_id);
    // if(!this->doc_.empty())
    // inst->doc_write(this->doc_);
    auto to_delete = unique_ptr<const InstBuilder>(this);
    return inst;
  }
} // namespace fhatos
