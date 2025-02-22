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

  Inst_p convert_to_inst(const Obj_p &lhs, const Inst_p &stub_inst, const Obj_p &obj) {
    if(obj->is_noobj() || obj->is_inst())
      return obj;
    //  LOG(INFO,"converting %s to inst\n",obj->toString().c_str());
    const Inst_p inst = InstBuilder::build("")
        ->inst_args(stub_inst->inst_args())
        ->inst_f(obj)
        ->domain_range(obj->domain(), obj->domain_coefficient(), obj->range(), obj->range_coefficient())
        ->create();
    LOG(TRACE, "converting %s to inst %s\n", obj->toString().c_str(), inst->toString().c_str());
    return inst;
  }

  Inst_p Compiler::resolve_inst(const Obj_p &lhs, const Inst_p &inst) const {
    //this->reset();
    if(inst->is_noobj())
      return inst;
    if(!lhs->is_noobj() && !this->coefficient_check(lhs->range_coefficient(), inst->domain_coefficient()))
      return Obj::to_noobj();
    Obj_p inst_obj = inst;
    if(!inst_obj->inst_f()) {
      // inst_vid
      if(inst->vid /*&& !inst->vid->is_relative()*/) {
        inst_obj = convert_to_inst(lhs, inst, Router::singleton()->read(*inst->vid));
        if(dt) dt->emplace_back(id_p(""), inst->vid, inst_obj);
      }
      // /obj_vid/::/inst_tid
      if((inst_obj->is_noobj() || !inst_obj->inst_f()) && lhs->vid) {
        inst_obj = convert_to_inst(
          lhs, inst, Router::singleton()->read(lhs->vid->add_component(*inst->tid)));
        if(dt) dt->emplace_back(lhs->vid, id_p(inst->tid), inst_obj);
      }
      // /obj_vid/::/resolved/inst_tid
      const ID inst_type_id_resolved = Router::singleton()->resolve(*inst->tid);
      if((inst_obj->is_noobj() || !inst_obj->inst_f()) && lhs->vid) {
        inst_obj = convert_to_inst(
          lhs, inst, Router::singleton()->read(lhs->vid->add_component(inst_type_id_resolved)));
        if(dt) dt->emplace_back(lhs->vid, id_p(inst_type_id_resolved), inst_obj);
      }
      // /obj_tid/::inst_tid
      if(inst_obj->is_noobj() || !inst_obj->inst_f()) {
        inst_obj = convert_to_inst(
          lhs, inst, Router::singleton()->read(lhs->tid->add_component(*inst->tid)));
        if(dt) dt->emplace_back(lhs->tid, inst->tid, inst_obj);
      }
      // /obj_tid/::/resolved/inst_tid
      if(inst_obj->is_noobj() || !inst_obj->inst_f()) {
        inst_obj = convert_to_inst(
          lhs, inst, Router::singleton()->read(lhs->tid->add_component(inst_type_id_resolved)));
        if(dt) dt->emplace_back(lhs->tid, id_p(inst_type_id_resolved), inst_obj);
      }
      // /resolved/inst_tid
      if(inst_obj->is_noobj() || !inst_obj->inst_f()) {
        inst_obj = convert_to_inst(lhs, inst, Router::singleton()->read(inst_type_id_resolved));
        if(dt) dt->emplace_back(id_p(""), id_p(inst_type_id_resolved), inst_obj);
      }
      // obj_tid/obj_tid (recurse)
      if(inst_obj->is_noobj() || !inst_obj->inst_f()) {
        if(const Obj_p parent = this->super_type(lhs); !parent->is_noobj()) {
          inst_obj = convert_to_inst(lhs, inst, resolve_inst(parent, inst));
        }
      }
      if(inst_obj->is_noobj() || !inst_obj->inst_f()) {
        if(!Router::singleton()->resolve(lhs->tid->no_query()).equals(*OBJ_FURI)) {
          inst_obj = convert_to_inst(lhs, inst, this->resolve_inst(
                                       Router::singleton()->read(
                                         Router::singleton()->read(
                                           Router::singleton()->resolve(
                                             lhs->tid->no_query()))->domain()->no_query()),
                                       inst_obj));
        }
      }
      if(this->throw_on_miss && (inst_obj->is_noobj() || !inst_obj->inst_f())) {
        string derivation_string;
        if(dt) this->print_derivation_tree(&derivation_string);
        else {
          const auto c = Compiler(false, true);
          c.resolve_inst(lhs, inst);
          c.print_derivation_tree(&derivation_string);
        }
        throw fError(FURI_WRAP_C(m) " !b%s!! !yinst!! unresolved %s", lhs->tid->toString().c_str(),
                     inst->tid->toString().c_str(), derivation_string.c_str());
      }
    }
    return inst_obj->is_inst() ? this->merge_inst(lhs, inst, inst_obj) : inst;
  }

  Inst_p Compiler::merge_inst(const Obj_p &lhs, const Inst_p &inst_provided, const Inst_p &inst_resolved) const {
    Inst_p inst_c;
    if(inst_resolved->is_inst()) {
      LOG(TRACE, "merging resolved inst into provide inst\n\t\t%s => %s [!m%s!!]\n",
          inst_resolved->toString().c_str(),
          inst_provided->toString().c_str(),
          "SIGNATURE HERE");
      const auto merged_args = Obj::to_inst_args();
      int counter = 0;
      for(const auto &[k,v]: *inst_resolved->inst_args()->rec_value()) {
        if(inst_provided->has_arg(k))
          merged_args->rec_value()->insert({k, inst_provided->arg(k)});
        else if(inst_provided->is_indexed_args() && counter < inst_provided->inst_args()->rec_value()->size())
          merged_args->rec_value()->insert({k, inst_provided->arg(counter)});
        else
          merged_args->rec_value()->insert({k, v->is_inst() ? v->arg(1) : v});
        // TODO: hack to get the default from from();
        ++counter;
      }
      // TODO: recurse off inst for all inst_arg getter/setters

      inst_c = Obj::to_inst(
        inst_resolved->inst_op(),
        merged_args,
        inst_resolved->inst_f(),
        inst_resolved->inst_seed_supplier(),
        inst_resolved->tid,
        inst_resolved->vid);
      /// TODO ^--- inst->vid);
    } else {
      inst_c = Obj::to_inst(
        inst_provided->inst_op(),
        inst_provided->inst_args(),
        make_shared<InstF>(make_shared<Cpp>(
          [x = inst_resolved->clone()](const Obj_p &lhs, const InstArgs &args) -> Obj_p {
            return x->apply(lhs, args);
          })),
        inst_provided->inst_seed_supplier(),
        inst_provided->tid,
        inst_provided->vid);
    }
    if(dt) this->dt->emplace_back(inst_resolved->tid, inst_c->tid, inst_c);
    LOG_OBJ(DEBUG, lhs, " !gresolved!! !yinst!! %s [!gEND!!]\n", inst_c->toString().c_str());
    return inst_c;
  }


  Obj_p Compiler::super_type(const Obj_p &value_obj) const {
    const Obj_p type_obj = Router::singleton()->read(value_obj->tid->no_query());
    if(type_obj->tid->no_query().equals(value_obj->tid->no_query())) {
      return Obj::to_noobj();
    }
    return type_obj;
  }

  bool Compiler::match(const Obj_p &lhs, const Obj_p &rhs) {
    // LOG(TRACE, "!ymatching!!: %s ~ %s\n", lhs->toString().c_str(), type->toString().c_str());
    if(rhs->is_empty_bcode())
      return true;
    if(rhs->is_type())
      return IS_TYPE_OF(lhs->tid, rhs->tid, {}) && !lhs->clone()->apply(rhs->type_value())->is_noobj();
    if(rhs->is_code() && !lhs->is_code()) {
      Obj_p result = rhs->apply(lhs->clone());
      if(result->is_noobj()) {
        if(rhs->range_coefficient().first == 0)
          return true;
        else return false;
      }
      return !result->is_noobj();
    }
    /* if(!rhs->value_.has_value() &&
        (rhs->tid->equals(*OBJ_FURI) || (FURI_OTYPE.count(rhs->tid->no_query()) && FURI_OTYPE.at(
                                                   rhs->tid->no_query()) == lhs->otype)))
       return true;*/
    if(lhs->otype != rhs->otype)
      return false;
    // if(require_same_type_id && (*lhs->tid != *rhs->tid))
    //  return false;
    switch(lhs->otype) {
      case OType::TYPE:
        return lhs->type_value()->match(rhs->is_type() ? rhs->type_value() : rhs);
      case OType::NOOBJ:
        return true;
      case OType::BOOL:
        return lhs->bool_value() == rhs->bool_value();
      case OType::INT:
        return lhs->int_value() == rhs->int_value();
      case OType::REAL:
        return lhs->real_value() == rhs->real_value();
      case OType::URI:
        return lhs->uri_value().matches(rhs->uri_value());
      case OType::STR:
        return lhs->str_value() == rhs->str_value();
      case OType::LST: {
        const auto objs_a = lhs->lst_value();
        const auto objs_b = rhs->lst_value();
        if(objs_a->size() != objs_b->size())
          return false;
        auto b = objs_b->begin();
        for(const auto &a: *objs_a) {
          if(!a->match(*b))
            return false;
          ++b;
        }
        return true;
      }
      case OType::REC: {
        const auto pairs_a = lhs->rec_value();
        const auto pairs_b = rhs->rec_value();
        for(const auto &[b_id, b_obj]: *pairs_b) {
          bool found = false;
          for(const auto &[a_id, a_obj]: *pairs_a) {
            if((b_id->is_uri() && b_id->uri_value().toString().find(':') != string::npos) || (
                 a_id->match(b_id) && a_obj->match(b_obj))) {
              found = true;
              break;
            }
          }
          if(!found)
            return false;
        }
        return true;
      }
      case OType::INST: {
        const auto args_a = lhs->inst_args();
        if(const auto args_b = rhs->inst_args(); !args_a->match(args_b))
          return false;
        if(lhs->domain_coefficient() != rhs->domain_coefficient() ||
           lhs->range_coefficient() != rhs->range_coefficient())
          return false;
        return true;
      }
      case OType::BCODE: {
        /*const auto insts_a = lhs->bcode_value();
        const auto insts_b = rhs->bcode_value();
        if (insts_a->size() != insts_b->size())
          return false;
        const auto b = insts_b->begin();
        for (const auto &a: *insts_a) {
          if (!a->match(*b))
            return false;
        }*/
        return true;
      }
      default:
        throw fError("unknown obj type in match(): %s", OTypes.to_chars(lhs->otype).c_str());
    }
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
    if(value_obj->is_noobj() && !type_id.equals(*NOOBJ_FURI))
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
      if(value_obj->match(type_obj, false))
        return true;
    } catch(const fError &) {
      // do nothing (fails below)
    }
    if(this->throw_on_miss) {
      static const auto p = GLOBAL_PRINTERS.at(value_obj->otype)->clone();
      p->show_type = false;
      Router::singleton()->log_frame_stack(ERROR);
      throw fError("!g[!b%s!g]!! %s is !rnot!! a !b%s!! as defined by %s", type_id.toString().c_str(),
                   value_obj->toString(p.get()).c_str(), type_id.toString().c_str(),
                   type_obj->toString().c_str());
    }
    return false;
  }


  Obj_p Compiler::apply_obj_to_inst(const Obj_p &source, const Inst_p &inst, const InstArgs &args) {
    /*if(!type_check(source,ROUTER_READ(inst->domain())))
      throw fError("type check error");
    const Obj_p sink = inst->apply(source,args);
    if(!type_check(sink, ROUTER_READ(inst->range())))
      throw fError("type check error");
    return sink;*/
    return source;
  }

  template<typename T>
  Coefficient<T> project_coefficient(const Coefficient<T> &source, const Coefficient<T> &sink) {
    if(source.first < sink.first)
      throw fError("out of range");
    if(source.second > sink.second)
      throw fError("out of range");
    return {source.first * sink.first, source.second * sink.second};
  }


  Obj_p Compiler::save_type(const ID_p &type_id, const Obj_p &type_obj) {
    return type_obj;
  }
};
