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
    static Types *singleton() {
      static Types factory = Types();
      TYPE_CHECKER = [](const Obj &obj, const OType otype, const fURI &typeId) {
        return Types::test(obj, otype, typeId);
      };
      return &factory;
    }
    static Option<Type_p> findType(const fURI &typeId) {
      Type_p type;
      Option<Type_p> typeOption = readFromCache(typeId);
      if (typeOption.has_value()) {
        return typeOption;
      } else {
        type = GLOBAL_OPTIONS->router<Router>()->read<Obj>(typeId);
        if (nullptr == type || nullptr == type.get())
          return Option<Type_p>();
        else {
          writeToCache(typeId, type, false);
          return Option<Type_p>(type);
        }
      }
    }
    static void writeToCache(const fURI &typeId, const Obj_p obj, const bool writeThrough = true) {
      TYPE_CACHE()->erase(typeId);
      TYPE_CACHE()->insert({typeId, obj});
      if (writeThrough)
        GLOBAL_OPTIONS->router<Router>()->write(obj, typeId);
    }
    static Option<Obj_p> readFromCache(const fURI &typeId, const bool readThrough = true) {
      if (TYPE_CACHE()->count(typeId))
        return Option<Obj_p>(TYPE_CACHE()->at(typeId));
      if (readThrough)
        return GLOBAL_OPTIONS->router<Router>()->read<Obj>(typeId);
      return Option<Obj_p>();
    }
    static bool test(const Obj &obj, const OType otype, const fURI &typeId, bool doThrow = true) noexcept(false) {
      const OType typeOType = STR_OTYPE.at(typeId.path(0, 1));
      if (typeOType == OType::INST || typeOType == OType::BCODE)
        return true;
      if (otype != typeOType) {
        if (doThrow)
          throw fError("Obj %s is not a %s\n", obj.toString().c_str(), typeId.toString().c_str());
        return false;
      }
      if (typeId.pathLength() == 2 && typeId.lastSegment().empty()) {
        return true;
      }
      Option<Type_p> type = findType(typeId);
      if (type.has_value()) {
        if (obj.match(*type, false)) {
          return true;
        }
        if (doThrow) {
          throw fError("Obj %s is not a %s\n", obj.toString().c_str(), typeId.toString().c_str());
        }
        return false;
      }

      if (doThrow) {
        throw fError("Undefined type %s\n", typeId.toString().c_str());
      }
      return false;
    }
  };
} // namespace fhatos
#endif


/*static Obj_p make(const Any value, const OType otype, const fURI_p &typeId) noexcept(false) {
    Obj_p obj = share(Obj(value, OTYPE_FURI.at(otype)));
    if (test(obj, typeId)) {
      return share(Obj(value, typeId));
    }
    return Obj::to_noobj();
  }
  static Bool_p make(const bool xbool, const fURI_p &typeId) { return make(xbool, OType::BOOL, typeId); }
  static Int_p make(const FL_INT_TYPE xint, const fURI_p &typeId) { return make(xint, OType::INT, typeId); }
  static Real_p make(const FL_REAL_TYPE xreal, const fURI_p &typeId) { return make(xreal, OType::REAL, typeId); }
  static Uri_p make(const fURI &xuri, const fURI_p &typeId) { return make(xuri, OType::URI, typeId); }
  static Str_p make(const string &xstr, const fURI_p &typeId) { return make(xstr, OType::STR, typeId); }
  static Rec_p make(const RecMap<> xmap, const fURI_p &typeId) { return make(xmap, OType::REC, typeId); }*/
