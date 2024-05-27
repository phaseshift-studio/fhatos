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

#ifndef fhatos_algebra_hpp
#define fhatos_algebra_hpp

#include <fhatos.hpp>
//
#include <language/obj.hpp>

namespace fhatos {
  class Algebra {
  public:
    inline static Algebra *singleton() {
      static Algebra singleton = Algebra();
      return &singleton;
    }

    /*
virtual const Obj* eq(const Obj*a,const Obj*b) const {
  return new Bool(*a == *b)
}

virtual const Obj* neq(const Obj*a,const Obj*b) const {
    return new Bool(!(*a == *b))
}

virtual const Obj* gt(const Obj*a,const Obj*b) const {

}

virtual const Obj* gte(const Obj*a,const Obj*b) const {

}

virtual const Obj* lt(const Obj*a,const Obj*b) const {

}

virtual const Obj* leq(const Obj*a,const Obj*b) const {

}
     */

    virtual const Obj *plus(const Obj *a, const Obj *b) const {
      switch (a->type()) {
        case OType::URI: return new Uri(((Uri *) b)->value().extend(((Uri *) a)->toString().c_str()));
        case OType::BOOL: return new Bool(((Bool *) b)->value() || ((Bool *) a)->value());
        case OType::INT: return new Int(((Int *) b)->value() + ((Int *) a)->value());
        case OType::REAL: return new Real(((Real *) b)->value() + ((Real *) a)->value());
        case OType::STR: return new Str(string(((Str *) b)->value().c_str()).append(((Str *) a)->value()));
        default: {
          throw fError("Algebra doesn't define %s + %s", OTYPE_STR.at(a->type()).c_str(),
                       OTYPE_STR.at(b->type()).c_str());
        }
      }
    }

    virtual const Obj *mult(const Obj *a, const Obj *b) const {
      switch (a->type()) {
        case OType::BOOL: return new Bool(((Bool *) b)->value() && ((Bool *) a)->value());
        case OType::INT: return new Int(((Int *) b)->value() * ((Int *) a)->value());
        case OType::REAL: return new Real(((Real *) b)->value() * ((Real *) a)->value());
        default: {
          throw fError("Algebra doesn't define %s * %s", OTYPE_STR.at(a->type()).c_str(),
                       OTYPE_STR.at(b->type()).c_str());
        }
      }
    }
  };

  //};
} // namespace fhatos

#endif
