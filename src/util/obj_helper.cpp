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
#include  "obj_helper.hpp"
#include "../lang/obj.hpp"
#include "../structure/router.hpp"

namespace fhatos {
  using std::make_pair;
  using std::vector;

     bool ObjHelper::check_coefficients(const IntCoefficient &a, const IntCoefficient &b, const bool throw_on_error ) {
      if(a.first < b.first || a.second > b.second) {
        if(throw_on_error) {
          throw fError("lhs coefficient not within rhs coefficient: {%i,%i} <> {%i,%i}", a.first, a.second, b.first,
                       b.second);
        }
        return false;
      }
      return true;
    }

     bool ObjHelper::check_noobj(const ID_p &type_id) {
      const vector<string> coef = type_id->query_values(FOS_RNG_COEF);
      return coef.empty() || stoi(coef.front()) == 0;
    }



     InstBuilder::InstBuilder(TypeO_p type) : type_(std::move(type)), args_(Obj::to_rec()), seed_(nullptr) {
    }


     InstBuilder * InstBuilder::build(const ID &type_id) {
      return new InstBuilder(id_p(Router::singleton()->resolve(type_id)));
    }

     InstBuilder *InstBuilder::build(const fURI_p &type_id) {
      return new InstBuilder(id_p(type_id));
    }


    InstBuilder *InstBuilder::inst_args(const Rec_p &args) {
      this->args_ = args;
      return this;
    }

    InstBuilder *InstBuilder::type_args(const Obj_p &arg0, const Obj_p &arg1 , const Obj_p &arg2 ,
                           const Obj_p &arg3 , const Obj_p &arg4 ) {
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

    InstBuilder *InstBuilder::domain_range(const ID_p &domain, const ID_p &range ) {
      return this->domain_range(domain, {1, 1}, range, {1, 1});
    }

    InstBuilder *InstBuilder::domain_range(const ID_p &domain, const IntCoefficient &domain_coefficient, const ID_p &range,
                              const IntCoefficient &range_coefficient) {
      this->type_ = id_p(this->type_->query({
        {FOS_DOMAIN, domain->toString()},
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
      this->function_supplier_ = make_shared<InstF>(make_shared<Cpp>(inst_f));
      return this;
    }

    InstBuilder *InstBuilder::inst_f(const Obj_p &obj) {
      this->function_supplier_ = make_shared<InstF>(obj);
      return this;
    }

    void InstBuilder::save(const Obj_p &root ) const {
      const Inst_p inst = this->create(nullptr, root);
      TYPE_SAVER(id_p(inst->vid_or_tid()->no_query()), inst);
    }

    Inst_p InstBuilder::create(const ID_p &value_id , const Obj_p &root) const {
      if(value_id) {
        if(const Inst_p maybe = Router::singleton()->read(*value_id); !maybe->is_noobj())
          return maybe;
      }
      const Inst_p inst = Inst::create(make_shared<InstValue>(make_tuple(
                                         this->args_,
                                         this->function_supplier_,
                                         this->seed_ ? this->seed_ : Obj::to_noobj())),
                                       OType::INST, this->type_,
                                       root ? id_p(root->vid->extend(*value_id)) : value_id);
      // if(!this->doc_.empty())
      // inst->doc_write(this->doc_);
      auto to_delete = unique_ptr<const InstBuilder>(this);
      return inst;
    }
} // namespace fhatos
