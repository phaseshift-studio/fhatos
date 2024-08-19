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
#include FOS_PROCESS(coroutine.hpp)
#include <structure/router.hpp>
#include <structure/stype/key_value.hpp>
#include <unistd.h>
#include <util/mutex_rw.hpp>

namespace fhatos {
  class Types : public Actor<Coroutine, KeyValue> {

    explicit Types(const ID &id = FOS_TYPE_PREFIX) : Actor(id) {}

  public:
    static ptr<Types> singleton(const ID &id = FOS_TYPE_PREFIX) {
      static ptr<Types> types_p = ptr<Types>(new Types(id));
      return types_p;
    }

    void setup() override {
      Actor::setup();
      TYPE_CHECKER = [](const Obj &obj, const OType otype, const ID_p &typeId) {
        singleton()->checkType(obj, otype, typeId, true);
        return typeId;
      };
      router()->route_subscription(share(
              Subscription{.source = *this->id(), .pattern = *this->pattern(), .onRecv = [](const Message_p &message) {
                if (message->retain && (message->source != *Types::singleton()->id()) &&
                    (message->source != ID("anon_src")))
                  Types::singleton()->saveType(id_p(message->target), message->payload, true);
                // else { // transient provides type checking?
                // TYPE_CHECKER(*message->payload, OTypes.toEnum(message->target.toString().c_str()),
                // id_p(message->target));
                //}
              }}));
    }

    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    void saveType(const ID_p &typeId, const Obj_p &typeDef, const bool viaPub = false) {
      try {
        if (!viaPub) {
          Obj_p current = this->read(typeId, this->id());
          if (!current->isNoObj() && current != typeDef) {
            LOG_PROCESS(WARN, this, "!b%s!g[!!%s!g] !ytype!! overwritten\n", typeId->toString().c_str(),
                        current->toString().c_str());
          }
          this->write(typeId, typeDef, this->id());
        }
        if (OType::INST == OTypes.toEnum(string(typeId->path(FOS_BASE_TYPE_INDEX)))) {
          const Inst_p inst = Insts::to_inst(*typeId, *typeDef->bcode_value());
          LOG_PROCESS(INFO, this, "!b%s!g[!!%s!g]!m:!b%s !ytype!! defined\n", typeId->toString().c_str(),
                      typeDef->bcode_value()->front()->toString().c_str(),
                      ITypeSignatures.toChars(inst->itype()).c_str());
        } else {
          LOG_PROCESS(INFO, this, "!b%s!g[!!%s!g] !ytype!! defined\n", typeId->toString().c_str(),
                      typeDef->toString().c_str());
        }
      } catch (const fError &e) {
        LOG_PROCESS(ERROR, this, "Unable to save type !b%s!!: %s\n", typeId->toString().c_str(), e.what());
      }
    }


    bool checkType(const Obj &obj, const OType otype, const ID_p &typeId, const bool doThrow = true) noexcept(false) {
      const OType typeOType = OTypes.toEnum(typeId->path(FOS_BASE_TYPE_INDEX));
      if (otype == OType::INST || otype == OType::BCODE || typeOType == OType::INST || typeOType == OType::BCODE)
        return true;
      if (otype != typeOType) {
        if (doThrow)
          throw fError("!g[!b%s!g]!! %s is not a !b%s!!\n", Types::singleton()->id()->toString().c_str(),
                       obj.toString().c_str(), typeId->toString().c_str());
        return false;
      }
      if (typeId->path_length() == (FOS_BASE_TYPE_INDEX + 1)) { // base type (otype)
        return true;
      }
      const Obj_p type = this->read(typeId, this->id());
      if (!type->isNoObj()) {
        if (obj.match(type, false)) {
          return true;
        }
        if (doThrow)
          throw fError("!g[!b%s!g]!! %s is not a !b%s!g[!!%s!g]!!\n", Types::singleton()->id()->toString().c_str(),
                       obj.toString().c_str(), typeId->toString().c_str(), type->toString().c_str());
        return false;
      }
      if (doThrow)
        throw fError("!g[!b%s!g] !b%s!! is an undefined !ytype!!\n", Types::singleton()->id()->toString().c_str(),
                     typeId->toString().c_str());
      return false;
    }
  };
} // namespace fhatos
#endif
