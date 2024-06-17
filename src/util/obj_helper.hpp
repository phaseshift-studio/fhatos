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

namespace fhatos {
  class ObjHelper final {
  public:
    ObjHelper() = delete;
    static const char *typeChars(const Obj *obj) { return OTYPE_STR.at(obj->type()); }

    static Option<fError> sameTypes(const Obj *a, const Obj *b) {
      return a->type() == b->type() && a->utype()->equals(*b->utype().get())
                 ? Option<fError>()
                 : Option<fError>(fError("Obj types are not equivalent: %s[%s] != %s[%s]",
                                         a->utype()->toString().c_str(), typeChars(a), b->utype()->toString().c_str(),
                                         typeChars(b)));
    }

    template <OType type,typename OBJ = Obj>
    static OBJ* checkType(const Obj* a) {
      if(a->type() != type)
        throw fError("Expected %s and received %s: %s\n",OTYPE_STR.at(type),typeChars(a),a->toString().c_str());
      return (OBJ*)a;
    }

    template <typename OBJ = Obj>
    static const OBJ* clone(const OBJ*obj) {
      switch(obj->type()) {
        case OType::BOOL: return (OBJ*) new Bool(*(Bool*)obj);
        case OType::INT: return (OBJ*) new Int(*(Int*)obj);
        case OType::REAL: return (OBJ*) new Real(*(Real*)obj);
        case OType::URI: return (OBJ*) new Uri(*(Uri*)obj);
        case OType::STR: return (OBJ*) new Str(*(Str*)obj);
        case OType::LST: return (OBJ*) new Lst(*(Lst*)obj);
        case OType::REC: return (OBJ*) new Rec(*(Rec*)obj);
        case OType::BYTECODE: return (OBJ*) new Bytecode(*(Bytecode*)obj);
        default: throw fError("Unable to clone obj of stype %s\n",obj->type());
      }
    }
    template <typename OBJ>
    static const ptr<const OBJ> clone_ptr(const ptr<const OBJ> obj) {
     return ptr<const OBJ>(ObjHelper::clone<const OBJ>(obj.get()));
    }
  };
} // namespace fhatos
#endif
