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

#ifndef fhatos_types_hpp
#define fhatos_types_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <process/router/router.hpp>
#include <util/options.hpp>


namespace fhatos {
  class Types {
  private:
    Types() = default;

  protected:
    static ptr<Map<fURI, Type_p>> TYPE_CACHE() {
      static ptr<Map<fURI, Type_p>> cache = share(Map<fURI, Type_p>());
      return cache;
    }

  public:
    enum class TYPE_SET { BASE, PROCESS };
    static void registerTypeSet(const TYPE_SET typeSet) {
      switch (typeSet) {
        case TYPE_SET::BASE:
          break;
        case TYPE_SET::PROCESS: {
          Types::writeToCache("/rec/thread",
                              Rec::to_rec({{u("setup"), *Obj::to_bcode({})}, {u("loop"), *Obj::to_bcode({})}}));
          Types::writeToCache("/rec/fiber",
                              Rec::to_rec({{u("setup"), *Obj::to_bcode({})}, {u("loop"), *Obj::to_bcode({})}}));
          break;
        }
      }
    }
    static Types *singleton() {
      static Types factory = Types();
      TYPE_CHECKER = [](const Obj &obj, const OType otype, const fURI &typeId) {
        return Types::test(obj, otype, typeId);
      };
      return &factory;
    }
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    static void writeToCache(const fURI &typeId, const Obj_p &obj, const bool writeThrough = true) {
      TYPE_CACHE()->erase(typeId);
      if (!obj->isNoObj()) {
        TYPE_CACHE()->insert({typeId, PtrHelper::clone<Obj>(obj)});
        if (writeThrough)
          GLOBAL_OPTIONS->router<Router>()->write(obj, typeId);
        LOG(INFO, "Type defined !b%s!!!g[!!%s!g]!!\n", typeId.toString().c_str(), obj->toString().c_str());
      }
    }
    static Option<Obj_p> readFromCache(const fURI &typeId, const bool readThrough = true) {
      if (TYPE_CACHE()->count(typeId) && !TYPE_CACHE()->at(typeId)->isNoObj())
        return Option<Obj_p>(TYPE_CACHE()->at(typeId));
      if (readThrough) {
        const Type_p type = GLOBAL_OPTIONS->router<Router>()->read<Obj>(typeId);
        return type->isNoObj() ? Option<Obj_p>() : Option<Obj_p>(type);
      }
      return Option<Obj_p>();
    }
    static bool test(const Obj &obj, const OType otype, const fURI &typeId, const bool doThrow = true) noexcept(false) {
      const OType typeOType = STR_OTYPE.at(typeId.path(0, 1));
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
      const Option<Type_p> type = readFromCache(typeId);
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
