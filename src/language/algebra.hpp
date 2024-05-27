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
  enum class COMPOSITION_OPERATOR {
    PLUS,
    MULT,
    MINUS,
    DIV,
    NEG
  };

  const static Map<COMPOSITION_OPERATOR, const char *> COMP_TO_STR = {
    {COMPOSITION_OPERATOR::PLUS, "plus"},
    {COMPOSITION_OPERATOR::MULT, "mult"},
    {COMPOSITION_OPERATOR::MINUS, "minus"},
    {COMPOSITION_OPERATOR::DIV, "div"},
    {COMPOSITION_OPERATOR::NEG, "neg"},
  };

  enum class RELATION_PREDICATE {
    EQ,
    NEQ,
    GT,
    GTE,
    LT,
    LTE
  };

  const static Map<RELATION_PREDICATE, const char *> REL_TO_STR = {
    {RELATION_PREDICATE::EQ, "eq"},
    {RELATION_PREDICATE::NEQ, "neq"},
    {RELATION_PREDICATE::GT, "gt"},
    {RELATION_PREDICATE::GTE, "gte"},
    {RELATION_PREDICATE::LT, "lt"},
    {RELATION_PREDICATE::LTE, "lte"}
  };


  class Algebra {
  public:
    virtual ~Algebra() = default;

    inline static Algebra *singleton() {
      static Algebra singleton = Algebra();
      return &singleton;
    }

    virtual auto relate(const RELATION_PREDICATE rel, const Obj *a, const Obj *b) const -> const Obj * {
      switch (rel) {
        case RELATION_PREDICATE::EQ: return new Bool(*a == *b);
        case RELATION_PREDICATE::NEQ: return new Bool(!(*a == *b));
        default: {
          throw fError("Algebra doesn't define %s + %s", OTYPE_STR.at(a->type()).c_str(),
                       OTYPE_STR.at(b->type()).c_str());
        }
      }
    }

    virtual auto compose(const COMPOSITION_OPERATOR comp, const Obj *a, const Obj *b) const -> const Obj * {
      switch (comp) {
        case COMPOSITION_OPERATOR::PLUS: {
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
        case COMPOSITION_OPERATOR::MULT: {
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
        default: {
          throw fError("Algebra does not support composition %s on %s", COMP_TO_STR.at(comp),
                       OTYPE_STR.at(a->type()).c_str(),
                       OTYPE_STR.at(b->type()).c_str());
        }
      }
    }
  };
} // namespace fhatos

#endif
