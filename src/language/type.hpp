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
#ifndef fhatos_types_hpp
#define fhatos_types_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>

#include FOS_MQTT(mqtt.hpp)

#define TOTAL_INSTRUCTIONS 75

namespace fhatos {
  using std::const_pointer_cast;

  class Type final : public Obj {
  public:
    ptr<ProgressBar> progress_bar_ = nullptr;

  protected:
    explicit Type(const ID &value_id, const ID &type_id) : Obj(share(RecMap<>(
                                                                 {
                                                                   /*{vri(":check"),
                                                                               Obj::to_inst([this](const Obj_p &lhs, const InstArgs &args) {
                                                                                              return dool(this->check_type(
                                                                                                  args.at(0).get(),
                                                                                                  furi_p(lhs->lst_value()->at(1)->uri_value())));
                                                                                            }, {x(0, ___)}, INST_FURI,
                                                                                            id_p(type_id.extend("inst/").extend(StringHelper::cxx_f_metadata(__FILE__,__LINE__))))},
                                                                              {vri(":start_progress_bar"),
                                                                               Obj::to_inst([this](const Int_p &, const InstArgs &args) {
                                                                                 this->start_progress_bar(args.at(0)->int_value());
                                                                                 return _noobj_;
                                                                               }, {x(0, ___)}, INST_FURI, make_shared<ID>(StringHelper::cxx_f_metadata(__FILE__,__LINE__)))},
                                                                              {vri(":end_progress_bar"),
                                                                               Obj::to_inst([this](const Str_p &, const InstArgs &args) {
                                                                                 this->end_progress_bar(args.at(0)->str_value());
                                                                                 return _noobj_;
                                                                               }, {x(0, ___)}, INST_FURI, make_shared<ID>(StringHelper::cxx_f_metadata(__FILE__,__LINE__)))},*/
                                                                 })),
                                                               OType::REC,
                                                               id_p(type_id),
                                                               id_p(value_id)) {
      ////////////////////////////////////////////////////////////////////////////////////////////////
      TYPE_SAVER = [this](const ID_p &type_id, const Obj_p &type_def) {
        this->save_type(type_id, type_def);
      };
      ////////////////////////////////////////////////////////////////////////////////////////////////
      TYPE_CHECKER = [this](const Obj *obj, const ID_p &type_id, const bool throw_on_fail) -> bool {
        if (type_id->equals(*OBJ_FURI) || type_id->equals(*NOOBJ_FURI))
          return true;
        const fURI_p resolved_type_id = resolve_shortened_base_type(obj->tid(), type_id);
        return this->check_type(obj, resolved_type_id, throw_on_fail);
      };
      ////////////////////////////////////////////////////////////////////////////////////////////////
      TYPE_MAKER = [this](const Obj_p &obj, const ID_p &type_id) -> Obj_p {
        const ID_p resolved_type_id = resolve_shortened_base_type(obj->tid(), type_id);
        const Obj_p type_def = ROUTER_READ(resolved_type_id);
        if (type_def->is_noobj()) {
          throw fError("!g[!b%s!g] !b%s!! is an undefined !ytype!!", this->vid()->toString().c_str(),
                       type_id->toString().c_str());
        }
        // TODO: require all type_defs be bytecode to avoid issue with type constant mapping ??
        const Obj_p proto_obj = type_id->equals(*OTYPE_FURI.at(obj->o_type())) || (
                                  !type_def->is_bcode() && !type_def->is_inst())
                                  ? obj
                                  : type_def->apply(obj);
        if (proto_obj->is_noobj() && !resolved_type_id->equals(*NOOBJ_FURI))
          throw fError("!g[!b%s!g]!! %s is not a !b%s!!", this->vid()->toString().c_str(), obj->toString().c_str(),
                       resolved_type_id->toString().c_str());
        return Obj::create(proto_obj->value_, obj->o_type(), resolved_type_id, obj->vid());
      };
      ///////////////////////////////////////////////////////////////
      RESOLVE_INST = [this](const Obj_p &this_obj, const ID_p &inst_type_id, List<ID> *derivation_tree) {
        Obj_p current_obj = this_obj;
        while (true) {
          if (current_obj->is_noobj())
            return noobj();
          const ID_p current_tid = current_obj->tid();
          ID_p current_vid = current_obj->vid();
          Inst_p maybe;
          if (current_vid) {
            LOG_OBJ(DEBUG, current_obj, "!b%s!m%s !yinst!! search\n",
                    current_vid->extend(C_INST_C).toString().c_str(),
                    inst_type_id->toString().c_str());
            if (derivation_tree)
              derivation_tree->emplace_back(current_vid->extend(C_INST_C).extend(*inst_type_id));
            maybe = ROUTER_READ(id_p(current_vid->extend(C_INST_C).extend(*inst_type_id)));
            if (!maybe->is_noobj())
              return maybe;
          }
          LOG_OBJ(DEBUG, current_obj, "!b%s!m%s !yinst!! search\n",
                  current_tid->extend(C_INST_C).toString().c_str(),
                  inst_type_id->toString().c_str());
          if (derivation_tree)
            derivation_tree->emplace_back(current_tid->extend(C_INST_C).extend(*inst_type_id));
          maybe = ROUTER_READ(id_p(current_tid->extend(C_INST_C).extend(*inst_type_id)));
          if (!maybe->is_noobj())
            return maybe;
          current_obj = ROUTER_READ(current_obj->tid());
          if (current_tid->equals(*current_obj->tid())) // infinite loop (i.e. base type)
            return noobj();
        }
      };
    }

  public:
    static ptr<Type> singleton(const ID &id = FOS_SCHEME "/type") {
      static auto types_p = ptr<Type>(new Type(id, *REC_FURI));
      return types_p;
    }

    void start_progress_bar(const uint16_t size) {
      this->progress_bar_ = ProgressBar::start(Options::singleton()->printer<Ansi<>>().get(), size);
    }

    void end_progress_bar(const string &message) {
      if (this->progress_bar_) {
        this->progress_bar_->end(message);
        this->progress_bar_ = nullptr;
      }
    }

    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    void save_type(const ID_p &type_id, const Obj_p &type_def) const {
      try {
        const Obj_p current = ROUTER_READ(type_id);
        if (this->progress_bar_) {
          ROUTER_WRITE(type_id, type_def,RETAIN);
          this->progress_bar_->incr_count(type_id->toString());
          if (this->progress_bar_->done())
            ROUTER_WRITE(this->vid(), const_pointer_cast<Obj>(shared_from_this()),RETAIN);
        } else {
          ROUTER_WRITE(type_id, type_def,RETAIN);
          if (current->is_noobj()) {
            LOG_OBJ(INFO, this, FURI_WRAP " !ytype!! defined\n",
                    type_id->toString().c_str(),
                    type_id->toString().c_str());
          } else {
            LOG_OBJ(INFO, this, "!b%s !ytype!! !b!-%s!! overwritten\n",
                    type_id->toString().c_str(), current->toString().c_str());
          }
        }
      } catch (const fError &e) {
        LOG_EXCEPTION(this, e);
      }
    }

    // syntax sugar hack to allow users to type 'int' instead of '/type/int/'
    static ID_p resolve_shortened_base_type(const fURI_p &type, const fURI_p &furi) {
      return OTypes.has_enum(furi->toString())
               ? OTYPE_FURI.at(OTypes.to_enum(furi->toString()))
               : id_p(type->resolve(*furi));
    }

    bool check_type(const Obj *obj, const fURI_p &type_id, const bool do_throw = true) const
      noexcept(false) {
      // if the type has already been associated with the object, then it's already been type checked TODO: is this true?
      if (obj->tid()->equals(*type_id))
        return true;
      // don't type check code yet -- this needs to be thought through more carefully as to the definition of code equivalence
      if (obj->o_type() == OType::OBJ || obj->o_type() == OType::INST || obj->o_type() == OType::BCODE)
        return true;
      if (type_id->equals(*NOOBJ_FURI) && obj->o_type() == OType::NOOBJ)
        return true;
      // if the type is a base type and the base types match, then type check passes
      if (type_id->equals(*OTYPE_FURI.at(obj->o_type())))
        return true;
      // get the type defintion and match it to the obj
      const Obj_p type = ROUTER_READ(type_id);
      if (!type->is_noobj()) {
        if (obj->match(type, false))
          return true;
        if (do_throw)
          throw fError("!g[!b%s!g]!! %s is not a !b%s!g[!!%s!g]!!", this->vid()->toString().c_str(),
                       obj->toString(false).c_str(), type_id->toString().c_str(), type->toString().c_str());
        return false;
      }
      if (do_throw)
        throw fError("!g[!b%s!g] !b%s!! is an undefined !ytype!!", this->vid()->toString().c_str(),
                     type_id->toString().c_str());
      return false;
    }
  };
} // namespace fhatos
#endif
