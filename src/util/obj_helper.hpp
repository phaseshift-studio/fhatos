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
  using LHSArgs = Pair<Obj_p, List_p<Obj_p>>;
  using LHSArgs_p = ptr<LHSArgs>;

  class ObjHelper final {
  public:
    ObjHelper() = delete;

    /* static Lst_p make_lhs_args(const Obj_p &lhs, const List<Obj_p> &args) {
       return lst({lhs, lst(make_shared<List<Obj_p>>(args))});
     }

     static LHSArgs_p parse_lhs_args(const Obj_p &maybe_lhs_pairs) {
       if(maybe_lhs_pairs->is_noobj())
         return make_shared<LHSArgs>(make_pair<Obj_p, List_p<Obj_p>>(noobj(), make_shared<List<Obj_p>>()));
       if(!maybe_lhs_pairs->is_lst())
         return make_shared<LHSArgs>(Pair<Obj_p, List_p<Obj_p>>(maybe_lhs_pairs, make_shared<List<Obj_p>>()));
       return make_shared<LHSArgs>(
         make_pair<Obj_p, List_p<Obj_p>>(maybe_lhs_pairs->lst_get(0), maybe_lhs_pairs->lst_get(1)->lst_value()));
     }

     static Obj_p apply_lhs_args(const Obj_p &old_obj, const Lst_p &lhs_args, const Obj_p &lhs = noobj()) {
       if(old_obj->is_noobj())
         throw fError("id doesn't not reference !ybcode!! or !yinst!!: !b%s!!", old_obj->toString().c_str());
       LOG(DEBUG, "apply_lhs_args: %s => %s\n", old_obj->toString().c_str(), lhs_args->toString().c_str());
       if(!lhs_args->is_lst())
         return old_obj->apply(lhs_args);
       if(lhs_args->is_lst() && lhs_args->lst_size()->int_value() == 1)
         return old_obj->apply(lhs_args->lst_get(0));
       if(lhs_args->is_lst() && lhs_args->lst_size()->int_value() > 1) {
         const Obj_p new_obj = ObjHelper::replace_from_obj(old_obj, *lhs_args->lst_get(1)->lst_value(), lhs);
         LOG(DEBUG, "structure read() transformed bcode: %s => %s\n", old_obj->toString().c_str(),
             new_obj->toString().c_str());
         return new_obj->apply(lhs_args->lst_get(0));
       }
       return old_obj;
     }

     static Inst_p replace_from_inst(const Obj_p &old_inst, const InstArgs &args, const Obj_p &lhs = noobj()) {
       const bool is_from = old_inst->inst_op() == "from";
       if(is_from && old_inst->inst_arg(0)->is_uri() && old_inst->inst_arg(0)->uri_value().toString()[0] == '_') {
         const uint8_t index = stoi(old_inst->inst_arg(0)->uri_value().name().substr(1));
         if(index < args.size())
           return args.at(index);
         if(old_inst->inst_args().size() == 2)
           return old_inst->inst_args().at(1); // default argument
         throw fError("%s requires !y%i!! arguments and !y%i!! were provided", old_inst->toString().c_str(),
                      old_inst->inst_args().size(), args.size());
       } else if(is_from && old_inst->inst_arg(0)->toString() == "_") {
         return lhs;
       } else {
         InstArgs new_args;
         for(const Obj_p &old_arg: old_inst->inst_args()) {
           new_args.push_back(replace_from_obj(old_arg, args, lhs));
         }
         return Obj::to_inst(old_inst->inst_op(), new_args, old_inst->inst_f(), old_inst->itype(),
                             old_inst->inst_seed_supplier(), old_inst->tid());
       }
     }

     static Obj_p replace_from_obj(const Obj_p &old_obj, const InstArgs &args, const Obj_p &lhs = noobj()) {
       if(old_obj->is_inst())
         return replace_from_inst(old_obj, args, lhs);
       else if(old_obj->is_bcode())
         return replace_from_bcode(old_obj, args, lhs);
       else if(old_obj->is_rec())
         return replace_from_rec(old_obj, args, lhs);
       else if(old_obj->is_lst())
         return replace_from_lst(old_obj, args, lhs);
       else
         return old_obj;
     }

     /*static Function<InstArgs, BCode_p> proto_bcode(const Obj_p &old_bcode) {
       return [old_bcode](const InstArgs &args) {
         return ObjHelper::replace_from_bcode(old_bcode, args);
       };
     }

     static BCode_p replace_from_bcode(const Obj_p &old_bcode, const InstArgs &args, const Obj_p &lhs = noobj()) {
       BCode_p new_bcode = Obj::to_bcode(make_shared<List<Obj_p>>(), old_bcode->tid());
       LOG(TRACE, "old bcode: %s\n", old_bcode->toString().c_str());
       for(const Inst_p &old_inst: *old_bcode->bcode_value()) {
         LOG(TRACE, "replacing old bcode inst: %s\n", old_inst->toString().c_str());
         const Inst_p new_inst = replace_from_inst(old_inst, args, lhs);
         new_bcode = new_bcode->add_inst(new_inst);
       }
       LOG(TRACE, "new bcode: %s\n", new_bcode->toString().c_str());
       return new_bcode;
     }

     static Rec_p replace_from_rec(const Obj_p &old_rec, const InstArgs &args, const Obj_p &lhs = noobj()) {
       Rec_p new_rec = rec();
       for(const auto &[key, value]: *old_rec->rec_value()) {
         new_rec->rec_set(replace_from_obj(key, args, lhs), replace_from_obj(value, args, lhs));
       }
       return new_rec;
     }

     static Lst_p replace_from_lst(const Obj_p &old_lst, const InstArgs &args, const Obj_p &lhs = noobj()) {
       Lst_p new_lst = lst();
       for(const auto &element: *old_lst->lst_value()) {
         new_lst->lst_add(replace_from_obj(element, args, lhs));
       }
       return new_lst;
     }
 */
    ////////////////////////////////////////////////

    static Rec_p encode_lst(const fURI &base_furi, const List<Obj_p> &list) {
      const Rec_p rec = Obj::to_rec();
      for(size_t i = 0; i < list.size(); i++) {
        rec->rec_set(vri(base_furi.resolve(string("./") + to_string(i))), list.at(i));
      }
      return rec;
    }
  };

  class InstBuilder {
    explicit InstBuilder(const TypeO_p &type) : type_(type), seed_(nullptr) {
    }

    /* InstF NO_OBJ_INST() const {
       return [this](const Obj_p &lhs, const InstArgs &args) {
         LOG(TRACE, "base-type instruction requiring resolution for %s\n", lhs->toString().c_str());

         List<ID> derivation = {*this->type_};
         Inst_p resolve = RESOLVE_INST(lhs, this->type_, &derivation);
         if(resolve->is_noobj()) {
           const Obj_p type_obj = ROUTER_READ(lhs->tid());
           derivation.push_back(*this->type_);
           resolve = RESOLVE_INST(lhs->as(lhs->type()->domain()), this->type_, &derivation);
         }
         if(resolve->is_noobj()) {
           string result;
           int counter = 0;
           for(int i = 0; i < derivation.size(); i++) {
             counter = derivation.at(i).equals(*this->type_) ? 1 : counter + 1;
             result.append(StringHelper::format("\t!m%s>" FURI_WRAP "\n",
                                                StringHelper::repeat(counter, "--").c_str(),
                                                derivation.at(i).toString().c_str()).c_str());
           }
           result = result.empty() ? "" : result.substr(0, result.size() - 1); // remove trailing \n
           throw fError(FURI_WRAP_C(m) " " FURI_WRAP " !yno inst!! resolution\n%s", lhs->tid()->toString().c_str(),
                        this->type_->toString().c_str(), result.c_str());
         }
         /* InstArgs args_applied;
                   for(const Obj_p &arg: args) {
                     args_applied.push_back(arg->apply(lhs));
                   }*/
    // return Obj::replace_from_obj(resolve, args, lhs)->apply(lhs);
    // return resolve->is_inst() ? resolve->inst_f()(lhs, args) : resolve->apply(lhs);
    // };*/
    //}

  protected:
    ID_p type_;
    InstArgs args_{Obj::to_inst_args()};
    ptr<InstF> function_supplier_ = nullptr;
    IType itype_{IType::ONE_TO_ONE};
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
      this->type_ = id_p(this->type_->query({
        {"domain", domain->toString()},
        {"range", nullptr == range ? domain->toString() : range->toString()}}));
      return this;
    }

    InstBuilder *itype_and_seed(const IType itype,
                                const Obj_p &seed = nullptr) {
      this->itype_ = itype;
      if(seed)
        this->seed_ = seed;
      else {
        this->seed_ = is_gather(this->itype_)
                        ? Obj::to_objs()
                        : _noobj_;
      }

      return this;
    }

    InstBuilder *doc(const string &documentation) {
      this->doc_ = documentation;
      return this;
    }

    InstBuilder *inst_f(const InstF &inst_f) {
      this->function_supplier_ = make_shared<InstF>(inst_f);
      return this;
    }

    InstBuilder *inst_f(const List_p<Inst_p> &code) {
      this->function_supplier_ = Obj::to_bcode(code);
      return this;
    }

    void save(const Obj_p &root = nullptr) const {
      const Inst_p inst = this->create(nullptr, root);
      TYPE_SAVER(id_p(inst->vid_or_tid()->query("")), inst);
    }

    [[nodiscard]] Inst_p create(const ID_p &value_id = nullptr, const Obj_p &root = nullptr) const {
      if(value_id) {
        if(const Inst_p maybe = ROUTER_READ(value_id); !maybe->is_noobj())
          return maybe;
      }
      const Inst_p inst = Inst::create(make_tuple(
                                         this->args_,
                                         this->function_supplier_,
                                         static_cast<IType>(this->itype_),
                                         this->seed_ ? this->seed_ : _noobj_),
                                       OType::INST, this->type_,
                                       root ? id_p(root->vid()->extend(*value_id)) : value_id);
      if(!this->doc_.empty())
        inst->doc_write(this->doc_);
      delete this;
      return inst;
    }
  };
} // namespace fhatos
#endif
