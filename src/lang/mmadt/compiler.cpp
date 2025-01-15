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

using std::tuple;
using std::vector;
using std::make_tuple;

namespace fhatos {
  // Obj_p compile(const Obj_p& starts, const BCode_p& bcode, const Algorithm compilation_algo);
  // Obj_p rewrite(const Obj_p& starts, const BCode_p& bcode, const vector<Inst_p>& rewrite_rules);
  // void explain(const Obj_p& starts, const BCode_p& bcode, const string* output);

  Compiler::Compiler() {
    this->throw_on_miss = true;
  }

  Inst_p Compiler::resolve_inst_to_id(const ID_p &vid_or_tid, const ID_p &inst_type_id, DerivationTree *dt) {
    const Inst_p maybe = ROUTER_READ(furi_p(vid_or_tid->add_component(*inst_type_id)));
    if(dt) dt->push_back({vid_or_tid, inst_type_id, maybe});
    return maybe;
  }


  Inst_p Compiler::resolve_inst(const Obj_p &source, const ID_p &inst_type_id, DerivationTree *dt) {
    const ID_p inst_type_id_resolved = id_p(*ROUTER_RESOLVE(fURI(*inst_type_id)));
    Obj_p inst_obj = ROUTER_READ(inst_type_id_resolved);
    if(inst_obj->is_noobj())
      inst_obj = resolve_inst_to_id(source->vid(), inst_type_id_resolved, dt);
    if(inst_obj->is_noobj())
      inst_obj = resolve_inst_to_id(source->tid(), inst_type_id_resolved, dt);
    if(inst_obj->is_noobj()) {
      Obj_p super = this->super_type(source);
      while(!super->is_noobj()) {
        inst_obj = resolve_inst_to_id(super->tid(), inst_type_id_resolved, dt);
      }
    }
    return inst_obj;
  }

  Obj_p Compiler::super_type(const Obj_p &value_obj) {
    Obj_p obj = ROUTER_READ(value_obj->tid());
    if(obj->is_noobj() || obj->equals(*value_obj)) {
      obj = ROUTER_READ(value_obj->domain());
    }
    if(obj->equals(*value_obj))
      return noobj();
    return obj;
  }

  bool Compiler::match(const Obj_p &lhs, const Obj_p &rhs, const DerivationTree *dt) {
    // LOG(TRACE, "!ymatching!!: %s ~ %s\n", lhs->toString().c_str(), type->toString().c_str());
    if(rhs->is_empty_bcode())
      return true;
    if(rhs->is_type())
      return IS_TYPE_OF(lhs->tid_, rhs->tid_, {}) && !lhs->clone()->apply(rhs->type_value())->is_noobj();
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
        (rhs->tid()->equals(*OBJ_FURI) || (FURI_OTYPE.count(rhs->tid()->no_query()) && FURI_OTYPE.at(
                                                   rhs->tid()->no_query()) == lhs->otype_)))
       return true;*/
    if(lhs->o_type() != rhs->o_type())
      return false;
    // if(require_same_type_id && (*lhs->tid_ != *rhs->tid_))
    //  return false;
    switch(lhs->o_type()) {
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
        throw fError("unknown obj type in match(): %s", OTypes.to_chars(lhs->o_type()).c_str());
    }
  }

  template<typename COEF>
  bool Compiler::coefficient_check(const COEF &lhs, const COEF &rhs) {
    if((lhs.first < rhs.first || lhs.second < rhs.first) || (lhs.first > rhs.second || lhs.second > rhs.second)) {
      if(this->throw_on_miss)
        throw fError("lhs coefficient not within rhs coefficient: {%i,%i} <> {%i,%i}", lhs.first, lhs.second, rhs.first,
                     rhs.second);

      return false;
    }
    return true;
  }


  bool Compiler::type_check(const Obj_p &value_obj, const ID_p &type_id, const DerivationTree *dt) {
    if(value_obj->is_noobj() && !type_id->equals(*NOOBJ_FURI))
      return false;
    if(type_id->equals(*OBJ_FURI) || type_id->equals(*NOOBJ_FURI)) // TODO: hack on noobj
      return true;
    // if the type is a base type and the base types match, then type check passes
    if(type_id->equals(*OTYPE_FURI.at(value_obj->o_type())))
      return true;
    // if the type has already been associated with the object, then it's already been type checked TODO: is this true?
    //if(value_obj->tid()->equals(*inst_type_id))
    //  return true;
    // don't type check code yet -- this needs to be thought through more carefully as to the definition of code equivalence
    if(value_obj->o_type() == OType::TYPE || value_obj->o_type() == OType::INST || value_obj->o_type() ==
       OType::BCODE)
      return true;
    if(type_id->equals(*NOOBJ_FURI) && (value_obj->o_type() == OType::NOOBJ || value_obj->tid()->
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
                     type_id->toString().c_str());
      return false;
    }
    if(!this->coefficient_check(value_obj->range_coefficient(), type_obj->domain_coefficient()))
      return false;
    try {
      if(type_obj->is_type() && !type_obj->apply(value_obj)->is_noobj())
        return true;
      if(value_obj->match(type_obj, false))
        return true;
    } catch(const fError &) {
      // do nothing (fails below)
    }
    if(this->throw_on_miss) {
      static const auto p = GLOBAL_PRINTERS.at(value_obj->o_type())->clone();
      p->show_type = false;
      throw fError("!g[!b%s!g]!! %s is !rnot!! a !b%s!! as defined by %s", type_id->toString().c_str(),
                   value_obj->toString(p.get()).c_str(), type_id->toString().c_str(),
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
