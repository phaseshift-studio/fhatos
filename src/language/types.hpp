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
#include <language/exts.hpp>
#include <language/obj.hpp>
#include <process/router/router.hpp>
#include <util/mutex_rw.hpp>
#include <util/options.hpp>

namespace fhatos {
  class Types {
  private:
    explicit Types() = default;

  protected:
    Map<fURI, Type_p> *CACHE = new Map<fURI, Type_p>();
    MutexRW<> *CACHE_MUTEX = new MutexRW<>();

  public:
    ~Types() {
      CACHE->clear();
      delete CACHE;
      delete CACHE_MUTEX;
    }
    static Types *singleton() {
      static bool _setup = false;
      static Types types = Types();
      if (!_setup) {
        TYPE_CHECKER = [](const Obj &obj, const OType otype, const ID &typeId) {
          singleton()->checkType(obj, otype, typeId, true);
          return ID_p(new ID(typeId));
        };
        TYPE_WRITER = [](const ID &id, const Type_p &type) {
          singleton()->saveType(id, type, true);
          return type;
        };
        TYPE_READER = [](const ID &typeId) { return singleton()->loadType(typeId, true).value_or(Obj::to_noobj()); };
        _setup = true;
      }
      return &types;
    }
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    void loadExt(const ID &extId) const {
      for (const Pair<ID, Type_p> &pair: Exts::exts(extId)) {
        this->saveType(pair.first, pair.second, true);
      }
    }
    const void saveType(const std::initializer_list<Pair<fURI, string>> &types, const bool writeThrough = true) const {
      for (const auto &pair: types) {
        saveType(pair.first, TYPE_PARSER(pair.second), writeThrough);
      }
    }
    const void saveType(const ID &typeId, const Obj_p &obj, const bool writeThrough = true) const {
      CACHE_MUTEX->write<void>([this, typeId, obj, writeThrough] {
        CACHE->erase(typeId);
        if (!obj->isNoObj()) {
          CACHE->insert({typeId, PtrHelper::clone<Obj>(obj)});
          if (writeThrough)
            GLOBAL_OPTIONS->router<Router>()->write(typeId, obj);
          LOG(INFO, "Type defined !b%s!!!g[!!%s!g]!!\n", typeId.toString().c_str(), obj->toString().c_str());
        }
        return share(nullptr);
      });
    }
    const Option<Obj_p> loadType(const ID &typeId, const bool readThrough = true) const {
      return CACHE_MUTEX->read<Option<Obj_p>>([this, typeId, readThrough] {
        if (CACHE->count(typeId) && !CACHE->at(typeId)->isNoObj())
          return Option<Obj_p>(CACHE->at(typeId));
        if (readThrough) {
          const Type_p type = GLOBAL_OPTIONS->router<Router>()->read<Obj>(typeId);
          return type->isNoObj() ? Option<Obj_p>() : Option<Obj_p>(type);
        }
        return Option<Obj_p>();
      });
    }
    // bool checkType(const Obj &obj, const OType otype, const Type_p type, const bool doThrow =true) const {}
    bool checkType(const Obj &obj, const OType otype, const ID &typeId, const bool doThrow = true) const
        noexcept(false) {
      const OType typeOType = OTypes.toEnum(typeId.path(0, 1).c_str());
      if (otype == OType::INST || otype == OType::BCODE || typeOType == OType::INST || typeOType == OType::BCODE)
        return true;
      if (otype != typeOType) {
        if (doThrow)
          throw fError("%s is not a !b%s!!\n", obj.toString().c_str(), typeId.toString().c_str());
        return false;
      }
      if (typeId.pathLength() == 2 && typeId.lastSegment().empty()) {
        return true;
      }
      const Option<Type_p> type = loadType(typeId);
      if (type.has_value()) {
        if (obj.match(*type, false)) {
          return true;
        }
        if (doThrow)
          throw fError("%s is not a !b%s!!\n", obj.toString().c_str(), typeId.toString().c_str());
        return false;
      }
      if (doThrow)
        throw fError("Undefined type %s\n", typeId.toString().c_str());
      return false;
    }
  };
} // namespace fhatos
#endif
