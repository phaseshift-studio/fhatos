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
    explicit Type(const ID &id = FOS_TYPE_PREFIX) :
      Obj(share(RecMap<>(
          {{vri(":check"),
            Obj::to_bcode([this](const Obj_p &obj) {
              return dool(this->check_type(
                  obj->lst_value()->at(0).get(),
                  furi_p(obj->lst_value()->at(1)->uri_value())));
            }, StringHelper::cxx_f_metadata(__FILE__,__LINE__))},
           {vri(":start_progress_bar"),
            Obj::to_bcode([this](const Int_p &obj) {
              this->start_progress_bar(obj->int_value());
              return noobj();
            }, StringHelper::cxx_f_metadata(__FILE__,__LINE__))},
           {vri(":end_progress_bar"),
            Obj::to_bcode([this](const Str_p &obj) {
              this->end_progress_bar(obj->str_value());
              return noobj();
            }, StringHelper::cxx_f_metadata(__FILE__,__LINE__))},
          })),
          OType::REC,
          REC_FURI,
          id_p(id)) {
      ////////////////////////////////////////////////////////////////////////////////////////////////
      TYPE_SAVER = [this](const ID_p &type_id, const Obj_p &type_def) {
        this->save_type(type_id, type_def);
      };
      ////////////////////////////////////////////////////////////////////////////////////////////////
      TYPE_CHECKER = [this](const Obj *obj, const ID_p &type_id, const bool throw_on_fail) -> bool {
        //const OType ztype = OTypes.to_enum(string(type_id->path(FOS_BASE_TYPE_INDEX)));
        if (type_id->equals(*MESSAGE_FURI) || type_id->equals(*SUBSCRIPTION_FURI))
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
        return make_shared<Obj>(proto_obj->value_, obj->o_type(), resolved_type_id, obj->vid());
      };
      ///////////////////////////////////////////////////////////////
      this->load_core_inst();
    }

    void load_core_inst() {
      INST_ARG = [](const uint8_t arg_num, const char *arg_name, const Obj_p &default_arg = noobj()) {
        return x(arg_num, arg_name, default_arg);
      };
      this->start_progress_bar(6);
      this->save_type(MESSAGE_FURI, Obj::to_rec({
                          {"target", Obj::to_bcode({Insts::as(vri(URI_FURI))})},
                          {"payload", Obj::to_bcode()},
                          {"retain", Obj::to_bcode({Insts::as(vri(BOOL_FURI))})}}));
      this->save_type(SUBSCRIPTION_FURI, Obj::to_rec({
                          {"source", Obj::to_bcode({Insts::as(vri(URI_FURI))})},
                          {"pattern", Obj::to_bcode({Insts::as(vri(URI_FURI))})},
                          {":on_recv", Obj::to_bcode()}}));
      this->save_type(THREAD_FURI, Obj::to_rec({{":loop", Obj::to_bcode()}}));
      this->save_type(HEAP_FURI, Obj::to_rec({{"pattern", Obj::to_bcode({Insts::as(vri(URI_FURI))})}}));
      this->save_type(MQTT_FURI, Obj::to_rec({
                          {"pattern", Obj::to_bcode({Insts::as(vri(URI_FURI))})},
                          {"broker", Obj::to_bcode({Insts::as(vri(URI_FURI))})},
                          {"client", Obj::to_bcode({Insts::as(vri(URI_FURI))})}}));
      this->end_progress_bar("!bfhatos !yobjs!! loaded\n");
    }

    static ID_p inst_id(const string &opcode) { return id_p(INST_FURI->resolve(opcode)); }

  public:
    static ptr<Type> singleton(const ID &id = FOS_TYPE_PREFIX) {
      static auto types_p = ptr<Type>(new Type(id));
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
          if (current->is_noobj()) {
            ROUTER_WRITE(type_id, type_def,RETAIN);
            LOG(INFO, FURI_WRAP " " FURI_WRAP " !ytype!! defined\n", this->vid()->toString().c_str(),
                type_id->toString().c_str(),
                type_id->toString().c_str());
          } else {
            ROUTER_WRITE(type_id, type_def,RETAIN);
            LOG(INFO, FURI_WRAP " " FURI_WRAP " !ytype!! !b!-%s!! overwritten\n", this->vid()->toString().c_str(),
                type_id->toString().c_str(), current->toString().c_str());
          }
        }
      } catch (const fError &e) {
        LOG_PROCESS(ERROR, this, "unable to save type !b%s!!: %s\n", type_id->toString().c_str(), e.what());
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
      if (obj->o_type() == OType::INST || obj->o_type() == OType::BCODE)
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