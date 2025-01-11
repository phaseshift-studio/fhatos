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

using std::tuple;
using std::vector;
using std::make_tuple;

namespace fhatos {
  // Obj_p compile(const Obj_p& starts, const BCode_p& bcode, const Algorithm compilation_algo);
  // Obj_p rewrite(const Obj_p& starts, const BCode_p& bcode, const vector<Inst_p>& rewrite_rules);
  // void explain(const Obj_p& starts, const BCode_p& bcode, const string* output);

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

  bool Compiler::type_check(const Obj_p &value_obj, const ID_p &inst_type_id, const DerivationTree *dt) {
    try {
      if(value_obj->is_noobj() && !inst_type_id->equals(*NOOBJ_FURI))
        return false;
      if(inst_type_id->equals(*OBJ_FURI) || inst_type_id->equals(*NOOBJ_FURI)) // TODO: hack on noobj
        return true;
      // if the type is a base type and the base types match, then type check passes
      if(inst_type_id->equals(*OTYPE_FURI.at(value_obj->o_type())))
        return true;
      // if the type has already been associated with the object, then it's already been type checked TODO: is this true?
      //if(obj->tid()->equals(*type_id))
      //  return true;
      // don't type check code yet -- this needs to be thought through more carefully as to the definition of code equivalence
      if(value_obj->o_type() == OType::TYPE || value_obj->o_type() == OType::INST || value_obj->o_type() ==
         OType::BCODE)
        return true;
      if(inst_type_id->equals(*NOOBJ_FURI) && (value_obj->o_type() == OType::NOOBJ || value_obj->tid()->
                                               equals(*OBJ_FURI)))
        return true;
      // get the type definition and match it to the obj
      if(const Obj_p type = ROUTER_READ(inst_type_id); !type->is_noobj()) {
        // ObjHelper::check_coefficients(value_obj->range_coefficient(), type->domain_coefficient());
        // if(type->is_type() && !obj->apply(type)->is_noobj())
        //   return true;
        if(value_obj->match(type, false))
          return true;
        if(nullptr != dt) {
          static const auto p = GLOBAL_PRINTERS.at(value_obj->o_type())->clone();
          p->show_type = false;
          throw fError("!g[!b%s!g]!! %s is !rnot!! a !b%s!! as defined by %s", inst_type_id->toString().c_str(),
                       value_obj->toString(p.get()).c_str(), inst_type_id->toString().c_str(),
                       type->toString().c_str());
        }
        return false;
      }
      if(nullptr != dt)
        throw fError("!g[!b%s!g] !b%s!! is an undefined !ytype!!", value_obj->vid_or_tid()->toString().c_str(),
                     inst_type_id->toString().c_str());
      return false;
    } catch(const fError &e) {
      return false;
    }
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
