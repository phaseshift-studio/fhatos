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

#ifndef fhatos_obj_helper_hpp
#define fhatos_obj_helper_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <language/otype/mono.hpp>

namespace fhatos {
  class ObjHelper final {
  public:
    ObjHelper() = delete;
    static const char *typeChars(const ptr<Obj> &obj) { return OTYPE_STR.at(obj->otype()); }

    static fError *sameTypes(const ptr<Obj> &a, const ptr<Obj> &b) {
      // LOG(DEBUG,"%s %s\n",a->toString().c_str(),b->toString().c_str());
      return a->otype() == b->otype() && a->type()->v_furi()->equals(*b->type()->v_furi())
                 ? nullptr
                 : new fError("Types are not equivalent: %s != %s\n", a->toString().c_str(), b->toString().c_str());
    }

    static ptr<Obj> cast(const Obj obj) {
      switch (obj.otype()) {
        case OType::BOOL:
          return ptr<Bool>(new Bool(*(Bool *) &obj));
        case OType::INT: {
          return ptr<Int>(new Int(*(Int *) &obj));
        }
        default:
          throw fError("Type not in cast: %s", OTYPE_STR.at(obj.otype()));
      }
    }

    template<OType otype, typename OBJ = Obj>
    static ptr<OBJ> checkType(const ptr<Obj> a) {
      if (a->otype() != otype)
        throw fError("Expected %s and received %s: %s\n", OTYPE_STR.at(otype), typeChars(a), a->toString().c_str());
      return ptr<OBJ>((OBJ *) a.get());
    }
    static const string objAnalysis(const ptr<Obj> obj, const string &value = "argument required") {
      char a[250];
      sprintf(a,
              "!b%s!! structure:\n"
              "\t!gotype!!         : %s/%i\n"
              "\t!gsize!!  (bytes) : %lu\n"
              "\t!gbcode!!         : %s\n"
              "\t!gvalue!!         : %s\n"
              "\t!gtype!!  (furi)  : %s",
              obj->type()->name().c_str(), OTYPE_STR.at(obj->otype()), (uint8_t) obj->otype(), sizeof(*obj),
              FOS_BOOL_STR(obj->isBytecode()), obj->isBytecode() ? obj->bcode()->toString().c_str() : value.c_str(),
              obj->type()->toString().c_str());
      return string(a);
    }

    template<typename OBJ = Obj>
    static OBJ *clone(const Obj *obj) {
      switch (obj->otype()) {
        case OType::BOOL:
          return (OBJ *) new Bool(*(Bool *) obj);
        case OType::INT:
          return (OBJ *) new Int(*(Int *) obj);
        case OType::REAL:
          return (OBJ *) new Real(*(Real *) obj);
        case OType::URI:
          return (OBJ *) new Uri(*(Uri *) obj);
        case OType::STR:
          return (OBJ *) new Str(*(Str *) obj);
        case OType::REC:
          return (OBJ *) new Rec(*(Rec *) obj);
        case OType::BYTECODE:
          return (OBJ *) new Bytecode(*(Bytecode *) obj);
        default:
          throw fError("Unable to clone obj of stype %s\n", obj->otype());
      }
    }
    template<typename OBJ>
    static const ptr<const OBJ> clone_ptr(const ptr<const OBJ> obj) {
      return ptr<const OBJ>(ObjHelper::clone<const OBJ>(obj.get()));
    }
  };
} // namespace fhatos
#endif
