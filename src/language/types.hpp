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
#include <process/actor/publisher.hpp>
#include FOS_PROCESS(coroutine.hpp)

#include <unistd.h>
#include <util/mutex_rw.hpp>

#ifndef FOS_USE_ROUTERS
#define FOS_USE_ROUTERS true
#endif

#ifdef FOS_USE_ROUTERS
#include <process/router/router.hpp>
#endif

namespace fhatos {
  class Types : public Coroutine, Publisher {

    explicit Types(const ID &id = ID("/type/")) : Coroutine(id), Publisher(this), CACHE_MUTEX(id.toString().c_str()) {}

  protected:
    Map<ID, Type_p> *CACHE = new Map<ID, Type_p>();
    MutexRW<> CACHE_MUTEX;

  public:
    ~Types() override {
      CACHE->clear();
      delete CACHE;
    }
    static Types *singleton(const ID &id = ID("/type/")) {
      static Types types = Types(id);
      return &types;
    }

    void setup() override {
      Coroutine::setup();
      TYPE_CHECKER = [](const Obj &obj, const OType otype, const ID_p &typeId) {
        singleton()->checkType(obj, otype, typeId, false);
        return typeId;
      };
      const List<Pattern> baseTypes = {BOOL_FURI->resolve("#"), INT_FURI->resolve("#"), REAL_FURI->resolve("#"),
                                       STR_FURI->resolve("#"),  URI_FURI->resolve("#"), LST_FURI->resolve("#"),
                                       REC_FURI->resolve("#"),  INST_FURI->resolve("#")};
      for (const Pattern &typeSub: baseTypes) {
        this->subscribe(typeSub, [](const Message_p &message) {
          try {
            if (!Types::singleton()->id()->equals(message->source))
              Types::singleton()->saveType(id_p(message->target), message->payload, false);
          } catch (const fError &e) {
            LOG_EXCEPTION(e);
          }
        });
      }
    }
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    void savePrefix(const char *prefix, const ID &furi, const bool writeThrough = true) {
      this->saveType(id_p(prefix), Uri::to_uri(furi), writeThrough);
    }

    Option<ID_p> loadPrefix(const char *prefix, const bool readThrough) {
      const Option<Obj_p> option = loadType(id_p(prefix), readThrough);
      if (option.has_value())
        return Option<ID_p>(id_p(option.value()->uri_value()));
      return Option<ID_p>();
    }

    void saveType(const ID_p &typeId, const Obj_p &typeDef, [[maybe_unused]] const bool writeThrough = true) {
      const ptr<bool> success = CACHE_MUTEX.write<bool>([this, typeId, typeDef, writeThrough] {
        try {
          if (CACHE->count(*typeId)) {
            LOG_TASK(WARN, this, "!b%s!g[!!%s!g]!m:!b%s !ytype!! overwritten\n", typeId->toString().c_str(),
                     CACHE->at(*typeId)->toString().c_str());
            CACHE->erase(*typeId);
          }
          if (!typeDef->isNoObj())
            CACHE->insert({ID(typeId->toString()), PtrHelper::clone<Obj>(typeDef)});
#if FOS_USE_ROUTERS
          if (writeThrough)
            this->publish(*typeId, typeDef, true);
#endif
        } catch (const fError &e) {
          LOG_TASK(ERROR, this, "Unable to save type !b%s!!: %s\n", typeId->toString().c_str(), e.what());
          return share(false);
        }
        return share(true);
      });
      if (*success) {
        if (OType::INST == OTypes.toEnum(typeId->path(0))) {
          const Inst_p inst = Insts::to_inst(*typeId, typeDef->bcode_value());
          LOG_TASK(INFO, this, "!b%s!g[!!%s!g]!m:!b%s !ytype!! defined\n", typeId->toString().c_str(),
                   typeDef->bcode_value().front()->toString().c_str(), ITypeSignatures.toChars(inst->itype()));
        } else {
          LOG_TASK(INFO, this, "!b%s!g[!!%s!g] !ytype!! defined\n", typeId->toString().c_str(),
                   typeDef->toString().c_str());
        }
      }
    }

    Option<Obj_p> loadType(const ID_p &typeId, [[maybe_unused]] const bool readThrough = true) {
      const auto opt = CACHE_MUTEX.read<Option<Obj_p>>(
          [this, typeId] { return CACHE->count(*typeId) ? Option<Obj_p>(CACHE->at(*typeId)) : Option<Obj_p>(); });
      if (opt.has_value())
        return opt;
      Type_p type =
#if FOS_USE_ROUTERS
          readThrough ? Router::read<Obj>(*typeId) :
#endif
                      Obj::to_noobj();
      return type->isNoObj() ? Option<Obj_p>() : Option<Obj_p>(type);
    }

    bool checkType(const Obj &obj, const OType otype, const ID_p &typeId, const bool doThrow = true) noexcept(false) {
      const OType typeOType = OTypes.toEnum(typeId->path(0));
      if (otype == OType::INST || otype == OType::BCODE || typeOType == OType::INST || typeOType == OType::BCODE)
        return true;
      if (otype != typeOType) {
        if (doThrow)
          throw fError("!g[!b%s!g]!! %s is not a !b%s!!\n", Types::singleton()->id()->toString().c_str(),
                       obj.toString().c_str(), typeId->toString().c_str());
        return false;
      }
      if (typeId->path_length() == 1) { // base type (otype)
        return true;
      }
      const Option<Type_p> type = loadType(typeId);
      if (type.has_value()) {
        if (obj.match(type.value(), false)) {
          return true;
        }
        if (doThrow)
          throw fError("!g[!b%s!g]!! %s is not a !b%s!g[!!%s!g]!!\n", Types::singleton()->id()->toString().c_str(),
                       obj.toString().c_str(), typeId->toString().c_str(), type.value()->toString().c_str());
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
