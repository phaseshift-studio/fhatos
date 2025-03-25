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

#include "compiler.hpp"
#include "../../util/obj_helper.hpp"
#include "../obj.hpp"
#include "../../structure/router.hpp"
#include "../../model/fos/util/log.hpp"

using std::tuple;
using std::vector;
using std::make_tuple;

namespace fhatos {
  // Obj_p compile(const Obj_p& starts, const BCode_p& bcode, const Algorithm compilation_algo);
  // Obj_p rewrite(const Obj_p& starts, const BCode_p& bcode, const vector<Inst_p>& rewrite_rules);
  // void explain(const Obj_p& starts, const BCode_p& bcode, const string* output);
  bool Compiler::boot_loading = true;

  Compiler::Compiler(const bool throw_on_miss, const bool with_derivation) {
    this->throw_on_miss = throw_on_miss;
    this->dt = with_derivation ? make_shared<DerivationTree>() : nullptr;
  }

  void Compiler::print_derivation_tree(string *derivation_string) const {
    derivation_string->clear();
    if(this->dt) {
      int max_0 = 0;
      int max_1 = 0;
      int max_2 = 0;
      for(const auto &oir: *this->dt) {
        int c = std::get<0>(oir)->toString().length();
        if(c > max_0)
          max_0 = c;
        c = std::get<1>(oir)->toString().length();
        if(c > max_1)
          max_1 = c;
        c = std::get<2>(oir)->toString().length();
        if(c > max_2)
          max_2 = c;
      }
      int counter = 0;
      // "\n\t!m%-8s!g[!b%-15s!g] !b%-30s!! !m=>!m !b%-35s!!",

      derivation_string->append(StringHelper::format(string("\n\t%")
                                                     .append("8").append("s !y%-")
                                                     .append(to_string(max_0)).append("s  !y%-")
                                                     .append(to_string(max_1)).append("s    !y%-")
                                                     .append(to_string(max_2)).append("s!!").c_str(),
                                                     "   ",
                                                     "lhs id",
                                                     "inst id",
                                                     "resolve obj"));
      for(const auto &oir: *this->dt) {
        counter = std::get<1>(oir)->empty() ? 0 : counter + 1;
        if(counter != 0) {
          string indent = StringHelper::repeat(counter, "-").append("!g>!!");
          derivation_string->append(StringHelper::format(string("\n\t!m%")
                                                         .append("8").append("s!g[!b%-")
                                                         .append(to_string(max_0)).append("s!g] !b%-")
                                                         .append(to_string(max_1)).append("s!! !m=>!m !b%-")
                                                         .append(to_string(max_2)).append("s!!").c_str(),
                                                         indent.c_str(),
                                                         std::get<0>(oir)->toString().c_str(),
                                                         std::get<1>(oir)->toString().c_str(),
                                                         std::get<2>(oir)->toString().c_str()));
        }
      }
    } else {
      derivation_string->append("<no derivation tree>");
    }
  }

  bool match_inst_args(const InstArgs &argsA, const InstArgs &argsB) {
    for(const auto &[kb,vb]: *argsB->rec_value()) {
      bool found = false;
      if(vb->is_noobj()) {
        for(const auto &[ka,va]: *argsA->rec_value()) {
          if(ka->match(kb))
            found = true;
        }
      } else
        found = true;
      if(!found)
        return false;
    }
    return true;
  }

  Inst_p Compiler::convert_to_inst(const Obj_p &lhs, const Inst_p &stub_inst, const Obj_p &inst_obj) const {
    if(inst_obj->is_noobj())
      return Obj::to_noobj();
    if(inst_obj->is_inst()) {
      const bool good = match_inst_args(stub_inst->inst_args(), inst_obj->inst_args());
      if(dt)
        dt->emplace_back(id_p(lhs->vid_or_tid()->no_query()), id_p(inst_obj->vid_or_tid()->no_query()), inst_obj);
      return good ? inst_obj : Obj::to_noobj();
    } else {
      const Inst_p inst = InstBuilder::build("")
          ->inst_args(stub_inst->inst_args())
          ->inst_f(inst_obj)
          ->domain_range(inst_obj->domain(), inst_obj->domain_coefficient(), inst_obj->range(),
                         inst_obj->range_coefficient())
          ->create();
      LOG_WRITE(TRACE, inst_obj.get(), L("converting {} to inst {}\n", inst_obj->toString(), inst->toString()));
      return inst;
    }
  }

  Inst_p Compiler::resolve_inst(const Obj_p &lhs, const Inst_p &inst) const {
    //this->reset();
    if(inst->is_noobj())
      return inst;
    if(!lhs->is_noobj() && !this->coefficient_check(lhs->range_coefficient(), inst->domain_coefficient()))
      return Obj::to_noobj();
    Obj_p inst_obj = inst;
    if(inst_obj->is_inst_stub()) {
      // inst_vid
      if(inst_obj->vid)
        inst_obj = convert_to_inst(lhs, inst, Router::singleton()->read(*inst->vid));
      // /obj_vid/::/inst_tid
      if(inst_obj->is_inst_stub() && lhs->vid)
        inst_obj = convert_to_inst(
            lhs, inst, Router::singleton()->read(lhs->vid->add_component(inst->tid->no_query())));
      // /obj_tid/::inst_tid
      if(inst_obj->is_inst_stub())
        inst_obj = convert_to_inst(
            lhs, inst, Router::singleton()->read(lhs->tid->add_component(inst->tid->no_query())));
      // /obj_vid/::/resolved/inst_tid
      const ID inst_type_id_resolved = Router::singleton()->resolve(inst->tid->no_query());
      if(inst_obj->is_inst_stub() && lhs->vid)
        inst_obj = convert_to_inst(
            lhs, inst, Router::singleton()->read(lhs->vid->add_component(inst_type_id_resolved)));
      // /obj_tid/::/resolved/inst_tid
      if(inst_obj->is_inst_stub())
        inst_obj = convert_to_inst(
            lhs, inst, Router::singleton()->read(lhs->tid->add_component(inst_type_id_resolved)));
      // /resolved/inst_tid
      if(inst_obj->is_inst_stub())
        inst_obj = convert_to_inst(lhs, inst, Router::singleton()->read(inst_type_id_resolved));
      // obj_tid/obj_tid (recurse)
      if(inst_obj->is_inst_stub()) {
        if(const Obj_p parent = this->super_type(lhs); !parent->is_noobj()) {
          inst_obj = convert_to_inst(lhs, inst, resolve_inst(parent, inst));
        }
      }
      if(inst_obj->is_inst_stub()) {
        if(!Router::singleton()->resolve(lhs->tid->no_query()).equals(*OBJ_FURI)) {
          inst_obj = convert_to_inst(lhs, inst, this->resolve_inst(
                                         Router::singleton()->read(
                                             Router::singleton()->read(
                                                 Router::singleton()->resolve(
                                                     lhs->tid->no_query()))->domain()->no_query()),
                                         inst_obj));
        }
      }
      if(this->throw_on_miss && inst_obj->is_inst_stub()) {
        string derivation_string;
        if(dt)
          this->print_derivation_tree(&derivation_string);
        else {
          const auto c = Compiler(false, true);
          c.resolve_inst(lhs, inst);
          c.print_derivation_tree(&derivation_string);
        }
        throw fError(FURI_WRAP_C(m) " !b%s!! !yinst!! unresolved %s", lhs->vid_or_tid()->toString().c_str(),
                     inst->vid_or_tid()->toString().c_str(), derivation_string.c_str());
      }
    }
    return inst_obj->is_inst() ? this->merge_inst(lhs, inst, inst_obj) : inst;
  }

  bool in_block_list(const string &op) {
    const std::vector<Uri_p> *blocker_list = ROUTER_READ(MMADT_PREFIX "inst/blockers")->lst_value().get();
    const bool found = blocker_list->end() != std::find_if(blocker_list->begin(), blocker_list->end(),
                                                           [&op](const Uri_p &u) {
                                                             return u->uri_value().name() == op;
                                                           });
    //LOG_WRITE(INFO, str("here").get(), L("blockers {} {}: {}\n", op, dool(found)->toString(),
    //                                     ROUTER_READ(MMADT_PREFIX "inst/blockers")->toString()));
    return found;

  }

  Inst_p Compiler::merge_inst(const Obj_p &lhs, const Inst_p &inst_provided, const Inst_p &inst_resolved) const {
    const auto inst_provided_args = inst_provided->inst_args();
    if(inst_resolved->is_inst()) {
      LOG_WRITE(TRACE, lhs.get(), L("merging resolved inst into provide inst\n\t\t{} => {}\n",
                                    inst_resolved->toString().c_str(),
                                    inst_provided->toString().c_str())          );
      Obj_p merged_args = Obj::to_inst_args();
      const auto inst_resolved_args = inst_resolved->inst_args();
      if(!inst_resolved_args->is_indexed_args() && !inst_provided_args->is_indexed_args()) {
        merged_args = inst_resolved_args->apply(inst_provided_args);
      } else {
        int r_counter = 0;
        for(const auto &[rk,rv]: *inst_resolved_args->rec_value()) {
          bool found = false;
          for(const auto &[lk,lv]: *inst_provided_args->rec_value()) {
            if(lk->match(rk)) {
              found = true;
              if((lv->is_inst() && in_block_list(lv->inst_op())) ||
                 in_block_list(inst_provided->inst_op())) {
                merged_args->rec_set(rk, lv);
              } else {
                merged_args->rec_set(rk, rv->apply(lv->apply(lhs)));
              }
            }
          }
          if(!found) {
            if(inst_provided_args->is_indexed_args() && r_counter < inst_provided_args->rec_value()->size())
              merged_args->rec_set(rk, rv->apply(inst_provided_args->arg(r_counter)->apply(lhs)));
            else
              merged_args->rec_set(rk, rv->apply(Obj::to_noobj()->apply(lhs)));
          }
          r_counter++;
        }
      }
      /*LOG(INFO, "inst_resolved: %s\ninst_provided: %s\nnew args: %s\n",
    inst_resolved_args->toString().c_str(),
    inst_provided_args->toString().c_str(),
    merged_args->toString().c_str());*/
      return Obj::to_inst(
          inst_resolved->inst_op(),
          merged_args,
          inst_resolved->inst_f(),
          inst_resolved->inst_seed_supplier(),
          inst_resolved->tid,
          inst_resolved->vid);
      /// TODO ^--- inst->vid);
    } else {
      return Obj::to_inst(
          inst_provided->inst_op(),
          inst_provided_args,
          InstF(make_shared<Cpp>(
              [x = inst_resolved->clone()](const Obj_p &lhs, const InstArgs &args) -> Obj_p {
                return x->apply(lhs, args);
              })),
          inst_provided->inst_seed_supplier(),
          inst_provided->tid,
          inst_provided->vid);
    }
  }


  Obj_p Compiler::super_type(const Obj_p &value_obj) const {
    Obj_p type_obj = Router::singleton()->read(value_obj->tid->no_query());
    if(type_obj->tid->no_query().equals(value_obj->tid->no_query())) {
      return Obj::to_noobj();
    }
    return type_obj;
  }

  template<typename COEF>
  bool Compiler::coefficient_check(const COEF &lhs, const COEF &rhs) const {
    if((lhs.first < rhs.first || lhs.second < rhs.first) || (lhs.first > rhs.second || lhs.second > rhs.second)) {
      if(this->throw_on_miss)
        throw fError("lhs coefficient not within rhs coefficient: {%i,%i} <> {%i,%i}", lhs.first, lhs.second, rhs.first,
                     rhs.second);

      return false;
    }
    return true;
  }

  bool Compiler::type_check(const Obj *value_obj, const ID &type_id) const {
    if(Compiler::boot_loading)
      return true;
    auto fail_reason = std::stack<string>();
    if(value_obj->is_noobj()) {
      if(const vector<string> coef = type_id.query_values(FOS_RNG_COEF);
        !coef.empty() && stoi(coef.front()) == 0) {
        return true;
      }
    }
    if(value_obj->is_rec()) {
      for(const auto &[k,v]: *value_obj->rec_value()) {
        if(k->is_uri() && k->uri_value().has_query()) {
          if(!this->type_check(v, k->uri_value().query()))
            return false;
        }
      }
    }
    const fURI type_no_query_id = type_id.no_query();
    if(type_no_query_id.equals(*OBJ_FURI) || type_no_query_id.equals(*NOOBJ_FURI)) // TODO: hack on noobj
      return true;
    // if the type is a base type and the base types match, then type check passes
    if(type_no_query_id.equals(*OTYPE_FURI.at(value_obj->otype)))
      return true;
    // if the type has already been associated with the object, then it's already been type checked TODO: is this true?
    // if(value_obj->tid->equals(type_no_query_id))
    //   return true;
    // don't type check code yet -- this needs to be thought through more carefully as to the definition of code equivalence
    if(value_obj->otype == OType::TYPE || value_obj->otype == OType::INST || value_obj->otype == OType::BCODE)
      return true;
    if(type_no_query_id.equals(*NOOBJ_FURI) && (value_obj->otype == OType::NOOBJ || value_obj->tid->equals(*OBJ_FURI)))
      return true;
    if(const Obj_p type = ROUTER_READ(type_no_query_id); !type->is_noobj()) {
      ObjHelper::check_coefficients(value_obj->range_coefficient(), type->domain_coefficient());
      // if(type->is_type() && !obj->apply(type)->is_noobj())
      //   return true;
      if(value_obj->match(type, &fail_reason))
        return true;
      if(this->throw_on_miss) {
        static const auto p = GLOBAL_PRINTERS.at(value_obj->otype)->clone();
        p->show_type = false;
        string fail;
        int count = 1;
        while(!fail_reason.empty()) {
          fail.append("\n\t\t")
              .append(StringHelper::repeat(count, " "))
              .append("!m\\")
              .append(StringHelper::repeat(count, "_"))
              .append("!!")
              .append(fail_reason.top());
          fail_reason.pop();
          count++;
        }
        throw fError("!g[!b%s!g]!! %s is !rnot!! a !b%s!! as defined by %s %s", "/sys/lang/parser",
                     value_obj->toString(p.get()).c_str(), type_id.toString().c_str(), type->toString().c_str(),
                     fail.c_str());
      }
      return false;
    }
    if(this->throw_on_miss)
      throw fError("!g[!b%s!g] !b%s!! is an undefined !ytype!!", "/sys/lang/parser", type_id.toString().c_str());
    return false;

    /*if(value_obj->is_noobj() && !type_id.equals(*NOOBJ_FURI))
      return false;
    if(type_id.equals(*OBJ_FURI) || type_id.equals(*NOOBJ_FURI)) // TODO: hack on noobj
      return true;
    // if the type is a base type and the base types match, then type check passes
    if(type_id.equals(*OTYPE_FURI.at(value_obj->otype)))
      return true;
    // if the type has already been associated with the object, then it's already been type checked TODO: is this true?
    //if(value_obj->tid->equals(*inst_type_id))
    //  return true;
    // don't type check code yet -- this needs to be thought through more carefully as to the definition of code equivalence
    if(value_obj->otype == OType::TYPE || value_obj->otype == OType::INST || value_obj->otype ==
       OType::BCODE)
      return true;
    if(type_id.equals(*NOOBJ_FURI) && (value_obj->otype == OType::NOOBJ || value_obj->tid->
                                       equals(*OBJ_FURI)))
      return true;
    // get the type definition and match it to the obj
    Obj_p type_obj;
    try {
      type_obj = Router::singleton()->read(type_id);
    } catch(const fError &) {
      type_obj = Obj::to_noobj();
    }
    if(type_obj->is_noobj()) {
      if(this->throw_on_miss)
        throw fError("!g[!b%s!g] !b%s!! is an undefined !ytype!!", value_obj->vid_or_tid()->toString().c_str(),
                     type_id.toString().c_str());
      return false;
    }
    if(!this->coefficient_check(value_obj->range_coefficient(), type_obj->domain_coefficient()))
      return false;
    try {
      if(type_obj->is_type() && !type_obj->apply(value_obj->shared_from_this())->is_noobj())
        return true;
      if(value_obj->match(type_obj, &fail_reason))
        return true;
    } catch(const fError &) {
      // do nothing (fails below)
    }
    if(this->throw_on_miss) {
      static const auto p = GLOBAL_PRINTERS.at(value_obj->otype)->clone();
      p->show_type = false;
      string fail;
      while(!fail_reason.empty()) {
        fail.append("\n").append(fail_reason.top());
        fail_reason.pop();
      }
      throw fError("!g[!b%s!g]!! %s is !rnot!! a !b%s!! as defined by %s %s", type_id.toString().c_str(),
                   value_obj->toString(p.get()).c_str(), type_id.toString().c_str(),
                   type_obj->toString().c_str(), fail.c_str());
    }
    return false;*/
  }

};
