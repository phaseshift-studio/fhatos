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
    static const char *typeChars(const ptr<Obj> &obj) { return OTYPE_STR.at(obj->o_type()); }

    static fError *sameTypes(const ptr<Obj> &a, const ptr<Obj> &b) {
      // LOG(DEBUG,"%s %s\n",a->toString().c_str(),b->toString().c_str());
      return (a->o_type() == b->o_type() &&
              a->o_type() == b->o_type() /*&& a->_furi->v_furi()->equals(*b->type()->v_furi()*/)
                 ? nullptr
                 : new fError("Types are not equivalent: %s != %s\n", a->toString().c_str(), b->toString().c_str());
    }

    template<OType O_DOMAIN, OType O_RANGE, typename OBJ = Obj>
    static ptr<OBJ> checkType(const ptr<Obj> a) {
      if (a->o_type() != O_DOMAIN && a->o_type() != O_RANGE)
        throw fError("Expected %s and received %s: %s\n", OTYPE_STR.at(O_RANGE), typeChars(a), a->toString().c_str());
      return a;
    }
    static const string objAnalysis(const Obj &obj) {
      char a[250];
      sprintf(a,
              "!b%s!! structure:\n"
              "\t!gid!!            : %s\n"
              "\t!grange<=domain!! : %s<=%s\n"
              "\t!gsize!!  (bytes) : %lu\n"
              "\t!gbcode!!         : %s\n"
              "\t!gvalue!!         : %s",
              obj.id()->lastSegment().c_str(), obj.id()->toString().c_str(), OTYPE_STR.at(obj.o_type()),
              OTYPE_STR.at(obj.o_type()), sizeof(obj), FOS_BOOL_STR(obj.isBytecode()),
              obj.isBytecode() ? obj.toString().c_str() : obj.toString(false).c_str());
      return string(a);
    }
  };
} // namespace fhatos
#endif
