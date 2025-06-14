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
#include "../../model/fos/sys/router/router.hpp"
#include "../../model/fos/util/log.hpp"
#include "../../util/obj_helper.hpp"
#include "../obj.hpp"

using std::make_tuple;
using std::tuple;
using std::vector;

namespace fhatos {
  // Obj_p compile(const Obj_p& starts, const BCode_p& bcode, const Algorithm compilation_algo);
  // Obj_p rewrite(const Obj_p& starts, const BCode_p& bcode, const vector<Inst_p>& rewrite_rules);
  // void explain(const Obj_p& starts, const BCode_p& bcode, const string* output);
  Compiler::Compiler(const bool throw_on_miss) : throw_on_miss(throw_on_miss), dt(nullptr) {}

  Compiler &Compiler::with_derivation_tree(DerivationTree *dt) {
    if(dt)
      this->dt = dt;
    else {
      auto dt1 = DerivationTree();
      this->dt = &dt1;
    }
    return *this;
  }

  void Compiler::print_derivation_tree(string *derivation_string) const {
    derivation_string->clear();
    if(this->dt) {
      size_t max_0 = 0;
      size_t max_1 = 0;
      size_t max_2 = 0;
      for(const auto &oir: *this->dt) {
        size_t c = std::get<0>(oir)->toString().length();
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
      derivation_string->append(StringHelper::format(string("\n\t%")
                                                         .append("8")
                                                         .append("s !y%-")
                                                         .append(to_string(max_0))
                                                         .append("s  !y%-")
                                                         .append(to_string(max_1))
                                                         .append("s    !y%-")
                                                         .append(to_string(max_2))
                                                         .append("s!!")
                                                         .c_str(),
                                                     "   ", "lhs id", "inst id", "resolve obj"));
      for(const auto &oir: *this->dt) {
        counter = std::get<1>(oir)->empty() ? 0 : counter + 1;
        if(counter != 0) {
          string indent = StringHelper::repeat(counter, "-").append("!g>!!");
          derivation_string->append(StringHelper::format(string("\n\t!m%")
                                                             .append("8")
                                                             .append("s!g[!b%-")
                                                             .append(to_string(max_0))
                                                             .append("s!g] !b%-")
                                                             .append(to_string(max_1))
                                                             .append("s!! !m=>!m !b%-")
                                                             .append(to_string(max_2))
                                                             .append("s!!")
                                                             .c_str(),
                                                         indent.c_str(), std::get<0>(oir)->toString().c_str(),
                                                         std::get<1>(oir)->toString().c_str(),
                                                         std::get<2>(oir)->toString().c_str()));
        }
      }
    } else {
      derivation_string->append("<no derivation tree>");
    }
  }

  bool match_inst_args(const InstArgs &provided_inst, const InstArgs &resolved_inst) {
    for(const auto &[kb, vb]: *resolved_inst->rec_value()) {
      bool found = false;
      if(vb->is_noobj()) {
        for(const auto &[ka, va]: *provided_inst->rec_value()) {
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

  Inst_p Compiler::convert_to_inst(const Obj_p &lhs, const Inst_p &provided_inst, const Obj_p &resolved_inst) const {
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // LOG_WRITE(INFO, lhs.get(),
    //           L("\n\t\tlhs\tprovided\tresolved\n\t\t{}\t{}\t{}\n", lhs->toString(), provided_inst->toString(),
    //             resolved_inst->toString()));
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(resolved_inst->is_noobj())
      return Obj::to_noobj();
    // TODO: obj{*} fails tests (fix)
    if(!resolved_inst->is_gather() && !Compiler(false).type_check(lhs, *resolved_inst->domain()))
      return Obj::to_noobj();
    Inst_p resolved = Obj::to_inst(
        InstValue(resolved_inst->is_inst() ? resolved_inst->inst_args() : provided_inst->inst_args(), // args
                  resolved_inst->is_inst() ? resolved_inst->inst_f() : resolved_inst, // inst_f
                  resolved_inst->is_inst() ? resolved_inst->inst_seed_supplier() : Obj::to_noobj()), // seed
        resolved_inst->is_inst() ? resolved_inst->tid : provided_inst->tid); // tid
    if(dt)
      dt->emplace_back(id_p(lhs->vid_or_tid()->no_query()), id_p(resolved->vid_or_tid()->no_query()), resolved);
    // necessary for reference instructions that simply redirect to another instruction
    if(resolved->is_inst_stub() && resolved->tid->name() != provided_inst->tid->name())
      resolved = this->resolve_inst(lhs, resolved);
    return match_inst_args(provided_inst->inst_args(), resolved->inst_args()) ? resolved : Obj::to_noobj();
  }

  Inst_p Compiler::resolve_inst(const Obj_p &lhs, const Inst_p &inst) const {
    // this->reset();
    if(inst->is_noobj())
      return inst;
    // if(!lhs->is_noobj() && !this->coefficient_check(lhs->range_coefficient(), inst->domain_coefficient()))
    // return Obj::to_noobj();
    Obj_p inst_obj = inst->inst_bcode_obj();
    inst_obj->resolve();
    //  inst_vid
    if(inst_obj->is_inst_stub() && inst_obj->vid)
      inst_obj = convert_to_inst(lhs, inst, Router::singleton()->read(*inst->vid));
    // obj field (inst tid)
    if(inst_obj->is_inst_stub()) {
      if(const Obj_p code = lhs->obj_get(*inst->tid)->inst_bcode_obj(); !code->is_noobj()) {
        inst_obj = convert_to_inst(lhs, inst, code);
      }
    }
    // obj field (inst name)
    if(inst_obj->is_inst_stub()) {
      if(const Obj_p code = lhs->obj_get(inst->tid->name())->inst_bcode_obj(); !code->is_noobj()) {
        inst_obj = convert_to_inst(lhs, inst, code);
      }
    }
    if(inst_obj->is_inst_stub() && inst_obj->vid) {
      if(const Obj_p code = lhs->obj_get(*inst->vid)->inst_bcode_obj(); code->is_code())
        inst_obj = convert_to_inst(lhs, inst, code);
    }

    // /obj_vid/::/inst_tid
    if(inst_obj->is_inst_stub() && lhs->vid)
      inst_obj = convert_to_inst(
          lhs, inst, Router::singleton()->read(lhs->vid->add_component(inst->tid->no_query()))->inst_bcode_obj());
    // /obj_tid/::/inst_tid
    if(inst_obj->is_inst_stub())
      inst_obj = convert_to_inst(
          lhs, inst, Router::singleton()->read(lhs->tid->add_component(inst->tid->no_query()))->inst_bcode_obj());
    // /obj_vid/::/resolved/inst_tid
    const ID inst_type_id_resolved = Router::singleton()->resolve(inst->tid->no_query());
    if(inst_obj->is_inst_stub() && lhs->vid)
      inst_obj = convert_to_inst(
          lhs, inst, Router::singleton()->read(lhs->vid->add_component(inst_type_id_resolved))->inst_bcode_obj());
    // /obj_tid/::/resolved/inst_tid
    if(inst_obj->is_inst_stub())
      inst_obj = convert_to_inst(
          lhs, inst, Router::singleton()->read(lhs->tid->add_component(inst_type_id_resolved))->inst_bcode_obj());
    // /resolved/inst_tid
    if(inst_obj->is_inst_stub())
      inst_obj = convert_to_inst(lhs, inst, Router::singleton()->read(inst_type_id_resolved)->inst_bcode_obj());
    // obj_tid/obj_tid (recurse)
    if(inst_obj->is_inst_stub()) {
      if(const Obj_p parent = this->super_type(lhs); !parent->is_noobj()) {
        inst_obj = convert_to_inst(lhs, inst, resolve_inst(parent, inst));
      }
    }
    if(inst_obj->is_inst_stub()) {
      if(!Router::singleton()->resolve(lhs->tid->no_query()).equals(*OBJ_FURI)) {
        inst_obj =
            convert_to_inst(lhs, inst,
                            this->resolve_inst(Router::singleton()
                                                   ->read(Router::singleton()
                                                              ->read(Router::singleton()->resolve(lhs->tid->no_query()))
                                                              ->domain()
                                                              ->no_query())
                                                   ->inst_bcode_obj(),
                                               inst_obj));
      }
    }
    if(this->throw_on_miss && inst_obj->is_inst_stub()) {
      string derivation_string;
      if(dt)
        this->print_derivation_tree(&derivation_string);
      else {
        DerivationTree *d = new DerivationTree();
        const auto c = Compiler(false).with_derivation_tree(d);
        c.resolve_inst(lhs, inst);
        c.print_derivation_tree(&derivation_string);
        delete d;
      }
      throw fError(FURI_WRAP_C(m) " !b%s!g(%s!g)!! !yinst!! unresolved %s", lhs->vid_or_tid()->toString().c_str(),
                   inst->vid_or_tid()->toString().c_str(), inst->inst_args()->toString().c_str(),
                   derivation_string.c_str());
    }
    return inst_obj->is_inst() ? this->merge_inst(lhs, inst, inst_obj) : inst;
  }

  bool Compiler::in_block_list(const string &op) {
    const Lst_p block_list_obj = Router::singleton()->read(MMADT_PREFIX "inst/blockers")->or_else(Obj::to_lst());
    const std::vector<Uri_p> *blocker_list = block_list_obj->lst_value().get();
    const bool found =
        blocker_list->end() != std::find_if(blocker_list->begin(), blocker_list->end(),
                                            [&op](const Uri_p &u) { return u->uri_value().name() == op; });
    // LOG_WRITE(INFO, str("here").get(), L("blockers {} {}: {}\n", op, dool(found)->toString(),
    //                                      Router::singleton()->read(MMADT_PREFIX "inst/blockers")->toString()));
    return found;
  }

  bool is_block_child(const Obj_p &obj) {
    return (obj->is_bcode() && !obj->bcode_value()->empty() &&
            Compiler::in_block_list(obj->bcode_value()->front()->inst_op())) ||
           (obj->is_inst() && Compiler::in_block_list(obj->inst_op()));
  }

  Obj_p get_child(const Obj_p &obj) {
    if(obj->is_bcode() && 1 == obj->bcode_value()->size() &&
       Compiler::in_block_list(obj->bcode_value()->front()->inst_op()) &&
       obj->bcode_value()->front()->inst_op() != "lift") {
      return obj->bcode_value()->front()->inst_args()->arg(0);
    } else if(obj->is_inst() && Compiler::in_block_list(obj->inst_op()) && obj->inst_op() != "lift") {
      return obj->inst_args()->arg(0);
    } else
      return obj;
  }

  bool is_else(const Obj_p &obj) {
    return (obj->is_not_empty_bcode() && obj->bcode_value()->front()->inst_op() == "else") ||
           (obj->is_inst() && obj->inst_op() == "else");
  }

  Inst_p Compiler::merge_inst(const Obj_p &lhs, const Inst_p &provided_inst, const Inst_p &resolved_inst) const {
    const auto inst_provided_args = provided_inst->inst_args();
    if(resolved_inst->is_inst()) {
      /* LOG_WRITE(TRACE, lhs.get(), L("merging resolved inst into provide inst\n\t\t{} => {}\n",
                                     resolved_inst->toString().c_str(),
                                     provided_inst->toString().c_str())          );*/
      Obj_p merged_args = Obj::to_inst_args();
      const auto inst_resolved_args = resolved_inst->inst_args();
      if(!inst_resolved_args->is_indexed_args() && !inst_provided_args->is_indexed_args()) {
        merged_args = inst_resolved_args->apply(inst_provided_args->apply(lhs));
      } else {
        int r_counter = 0;
        for(const auto &[rk, rv]: *inst_resolved_args->rec_value()) {
          bool found = false;
          for(const auto &[lk, lv]: *inst_provided_args->rec_value()) {
            if(lk->match(rk)) {
              found = true;
              merged_args->rec_set(rk, in_block_list(provided_inst->inst_op()) ? lv
                                       : is_block_child(lv)                    ? get_child(lv)
                                                                               : rv->apply(lv->apply(lhs)));
            }
          }
          if(!found)
            /* LOG_WRITE(INFO, rv.get(),L("\t{}\n\t{}\n\t{} ({},{})\n", rv->toString(), rv->type()->toString(),
                                        rv->domain()->toString(), rv->domain_coefficient().first,
               rv->domain_coefficient().second));*/
            merged_args->rec_set(
                rk, inst_provided_args->is_indexed_args() && r_counter < inst_provided_args->rec_value()->size()
                        ? in_block_list(provided_inst->inst_op()) || is_block_child(inst_provided_args->arg(r_counter))
                              ? inst_provided_args->arg(r_counter)
                              : rv->apply(inst_provided_args->arg(r_counter)->apply(lhs))
                        : rv->apply(is_else(rv) ? Obj::to_noobj() // TODO: use pre-computed inst domain coefficent
                                                : lhs)); // default arg
          r_counter++;
        }
      }
      /*LOG(TRACE, "**** %s ****\ninst_resolved args: %s\ninst_provided args: %s\ninst_merged args: %s\n",
          inst_resolved_args->vid_or_tid()->toString().c_str(),
          inst_resolved_args->toString().c_str(),
          inst_provided_args->toString().c_str(),
          merged_args->toString().c_str());*/
      return Obj::to_inst(resolved_inst->inst_op(), merged_args, resolved_inst->inst_f(),
                          resolved_inst->inst_seed_supplier(), resolved_inst->tid, provided_inst->vid);
    } else {
      return Obj::to_inst(
          provided_inst->inst_op(), inst_provided_args,
          InstF(make_shared<Cpp>(
              [x = resolved_inst->clone()](const Obj_p &lhs, const InstArgs &) -> Obj_p { return x->apply(lhs); })),
          provided_inst->inst_seed_supplier(), provided_inst->tid, provided_inst->vid);
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
    if((lhs.first < rhs.first && lhs.second < rhs.first) || lhs.second > rhs.second) {
      if(this->throw_on_miss)
        throw fError("lhs coefficient not within rhs coefficient: {%i,%i} <> {%i,%i}", lhs.first, lhs.second, rhs.first,
                     rhs.second);

      return false;
    }
    return true;
  }

  bool Compiler::type_check(const Obj *obj, const ID &type_id) const {
    if(BOOTING)
      return true;
    // don't type check code yet -- this needs to be thought through more carefully as to the definition of code
    // equivalence
    if(obj->otype == OType::TYPE || obj->otype == OType::INST || obj->otype == OType::BCODE)
      return true;
    // if(obj->is_inst())
    // return this->type_check(Router::singleton()->read(*obj->range()),type_id);
    if(obj->is_noobj()) {
      if(const vector<string> coef = type_id.query_values(FOS_RNG_COEF); !coef.empty() && stoi(coef.front()) == 0) {
        return true;
      }
    }
    if(obj->is_rec()) {
      for(const auto &[k, v]: *obj->rec_value()) {
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
    if(type_no_query_id.equals(*OTYPE_FURI.at(obj->otype)))
      return true;
    // if the type has already been associated with the object, then it's already been type checked TODO: is this true?
    // if(obj->tid->equals(type_no_query_id))
    //   return true;
    if(type_no_query_id.equals(*NOOBJ_FURI) && (obj->otype == OType::NOOBJ || obj->tid->equals(*OBJ_FURI)))
      return true;
    if(const Obj_p type = Router::singleton()->read(type_no_query_id); !type->is_noobj()) {
      Compiler::coefficient_check(obj->range_coefficient(), type->domain_coefficient());
      //  ObjHelper::check_coefficients(value_obj->range_coefficient(), type->domain_coefficient());
      // if(type->is_type() && !obj->apply(type)->is_noobj())
      //   return true;
      auto fail_reason = std::stack<string>();
      if(obj->match(type, &fail_reason))
        return true;
      if(this->throw_on_miss) {
        static const auto p = GLOBAL_PRINTERS.at(obj->otype)->clone();
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
        throw fError("!g[!b%s!g]!! %s is !rnot!! a !b%s!! as defined by %s %s", "/mmadt/parser",
                     obj->toString(p.get()).c_str(), type_id.toString().c_str(), type->toString().c_str(),
                     fail.c_str());
      }
      return false;
    }
    if(this->throw_on_miss)
      throw fError("!g[!b%s!g] !b%s!! is an undefined !ytype!!", "/mmadt/parser", type_id.toString().c_str());
    return false;
  }

}; // namespace fhatos
