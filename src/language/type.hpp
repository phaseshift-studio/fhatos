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
#include <language/insts.hpp>
#include <language/obj.hpp>
#include <process/obj_process.hpp>
#include <structure/router.hpp>
#include FOS_MQTT(mqtt.hpp)

#define TOTAL_INSTRUCTIONS 75

namespace fhatos {
  class Type final : public Coroutine {
  public:
    ptr<ProgressBar> progress_bar_ = nullptr;

  protected:
    explicit Type(const ID &id = FOS_TYPE_PREFIX) : Coroutine(id) {
    }

    static ID_p inst_id(const string &opcode) { return id_p(INST_FURI->resolve(opcode)); }

  public:
    static ptr<Type> singleton(const ID &id = FOS_TYPE_PREFIX) {
      static auto types_p = ptr<Type>(new Type(id));
      return types_p;
    }

    void setup() override {
      Coroutine::setup();
      TYPE_CHECKER = [](const Obj &obj, const fURI_p &type_id) -> void {
        //const OType ztype = OTypes.to_enum(string(type_id->path(FOS_BASE_TYPE_INDEX)));
        const fURI_p resolved_type_id = resolve_sugar_type(obj.type(), type_id);
        singleton()->check_type(obj, resolved_type_id, true);
      };
      TYPE_MAKER = [this](const Obj_p &obj, const ID_p &type_id) -> Obj_p {
        const ID_p resolved_type_id = resolve_sugar_type(obj->type(), type_id);
        if (OTypes.to_enum(resolved_type_id->path(FOS_BASE_TYPE_INDEX)) != obj->o_type())
          throw fError("!g[!b%s!g]!! %s is not a !b%s!!", this->id()->toString().c_str(), obj->toString().c_str(),
                       resolved_type_id->toString().c_str());
        const Obj_p type_def = router()->read(resolved_type_id);
        // TODO: require all type_defs be bytecode to avoid issue with type constant mapping
        const Obj_p proto_obj = is_base_type(resolved_type_id) || (!type_def->is_bcode() && !type_def->is_inst())
                                  ? obj
                                  : type_def->apply(obj);
        if ((proto_obj->is_noobj() && !resolved_type_id->equals(*NOOBJ_FURI)))
          throw fError("!g[!b%s!g]!! %s is not a !b%s!!", this->id()->toString().c_str(), obj->toString().c_str(),
                       resolved_type_id->toString().c_str());
        return make_shared<Obj>(proto_obj->_value, resolved_type_id, obj->id());
      };
      router()->route_subscription(
        subscription_p(ID(*this->id()), *this->id(), Insts::to_bcode([this](const Message_p &message) {
          const ID_p type_id = id_p(message->target);
          if (message->retain && !this->type_exists(type_id, message->payload))
            this->save_type(type_id, message->payload, true);
        })));
    }

    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    void save_type(const ID_p &type_id, const Obj_p &type_def, const bool via_pub = false) const {
      try {
        if (!via_pub) {
          const Obj_p current = router()->read(type_id);
          if (current != type_def) {
            if (!current->is_noobj() && !this->progress_bar_)
              LOG_PROCESS(WARN, this, "!b%s!g[!!%s!g] !ytype!! overwritten\n", type_id->toString().c_str(),
                        current->toString().c_str());
            router()->write(type_id, type_def, RETAIN_MESSAGE);
          }
        }
        if (!this->progress_bar_)
          LOG_PROCESS(INFO, this, "!b%s!g[!!%s!g] !ytype!! defined\n", type_id->toString().c_str(),
                    type_def->toString().c_str());
        else {
          this->progress_bar_->incr_count(type_id->toString());
        }
      } catch (const fError &e) {
        LOG_PROCESS(ERROR, this, "unable to save type !b%s!!: %s\n", type_id->toString().c_str(), e.what());
      }
    }

    bool type_exists(const ID_p &type_id, const Obj_p &type_def) const {
      const Obj_p existing_type_def = router()->read(type_id);
      return !existing_type_def->is_noobj() && (*existing_type_def == *type_def);
    }

    static ID_p resolve_sugar_type(const fURI_p &type, const fURI_p &furi) {
      return OTypes.has_enum(furi->toString())
               ? id_p(ID(string(FOS_TYPE_PREFIX) + furi->name()))
               : id_p(type->resolve(*furi));
    }

    static bool is_base_type(const ID_p &type_id) { return type_id->path_length() == FOS_BASE_TYPE_INDEX + 1; }

    bool check_type(const Obj &obj, const fURI_p &type_id, const bool do_throw = true) const
      noexcept(false) {
      const OType type_otype = OTypes.to_enum(string(type_id->path(FOS_BASE_TYPE_INDEX)));
      if (obj.o_type() == OType::INST || obj.o_type() == OType::BCODE || type_otype == OType::INST || type_otype ==
          OType::BCODE)
        return true;
      if (obj.o_type() != type_otype) {
        if (do_throw)
          throw fError("!g[!b%s!g]!! %s is not a !b%s!!", this->id()->toString().c_str(), obj.toString(false).c_str(),
                       type_id->toString().c_str());
        return false;
      }
      if (type_id->path_length() == (FOS_BASE_TYPE_INDEX + 1)) {
        // base type (otype)
        return true;
      }
      const Obj_p type = router()->read(type_id);
      if (!type->is_noobj()) {
        if (obj.match(type, false)) {
          return true;
        }
        if (do_throw)
          throw fError("!g[!b%s!g]!! %s is not a !b%s!g[!!%s!g]!!", this->id()->toString().c_str(),
                       obj.toString(false).c_str(), type_id->toString().c_str(), type->toString().c_str());
        return false;
      }
      if (do_throw)
        throw fError("!g[!b%s!g] !b%s!! is an undefined !ytype!!", this->id()->toString().c_str(),
                     type_id->toString().c_str());
      return false;
    }
  };
} // namespace fhatos
#endif
