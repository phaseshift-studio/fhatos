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
    virtual ~Algebra() = default;

    static Algebra *singleton() {
      static Algebra singleton = Algebra();
      return &singleton;
    }

    /////////////////////////////////////////////////////////////////////
    ////////////////////////////// CONTROL //////////////////////////////
    /////////////////////////////////////////////////////////////////////

    enum class BRANCH_SEMANTIC {
      SPLIT,
      CHAIN,
      SWITCH
    };

    static const char *BRNCH_TO_STR(const BRANCH_SEMANTIC brnch) {
      const static Map<const BRANCH_SEMANTIC, const char *> map = {
        {BRANCH_SEMANTIC::SPLIT, "split"},
        {BRANCH_SEMANTIC::CHAIN, "chain"},
        {BRANCH_SEMANTIC::SWITCH, "switch"}
      };
      return map.at(brnch);
    }

    virtual const Obj *branch(const BRANCH_SEMANTIC branch, const Obj *incoming, const Obj *control) const {
      switch (branch) {
        case BRANCH_SEMANTIC::SPLIT: {
          return NoObj::singleton();
        }
        case BRANCH_SEMANTIC::CHAIN: {
          return NoObj::singleton();
        }
        case BRANCH_SEMANTIC::SWITCH: {
          for (const auto &[check, outgoing]: *((Rec *) control)->value()) {
            if (!check->apply(incoming)->isNoObj()) {
              return outgoing->apply(incoming);
            }
          }
          return NoObj::singleton();
        }
        default: {
          throw fError("Algebra doesn't define %s + %s", OTYPE_STR.at(incoming->type()),
                       OTYPE_STR.at(control->type()));
        }
      }
    }

    ///////////////////////////////////////////////////////////////////
    //////////////////////////// RELATIONS ////////////////////////////
    ///////////////////////////////////////////////////////////////////

    enum class RELATION_PREDICATE {
      EQ,
      NEQ,
      GT,
      GTE,
      LT,
      LTE
    };

    static const char *REL_TO_STR(const RELATION_PREDICATE rel) {
      const static Map<const RELATION_PREDICATE, const char *> map = {
        {RELATION_PREDICATE::EQ, "eq"},
        {RELATION_PREDICATE::NEQ, "neq"},
        {RELATION_PREDICATE::GT, "gt"},
        {RELATION_PREDICATE::GTE, "gte"},
        {RELATION_PREDICATE::LT, "lt"},
        {RELATION_PREDICATE::LTE, "lte"}
      };
      return map.at(rel);
    }

    virtual const Obj *relate(const RELATION_PREDICATE rel, const Obj *a, const Obj *b) {
      switch (rel) {
        case RELATION_PREDICATE::EQ: return new Bool(*(Obj *) a == *(Obj *) b);
        case RELATION_PREDICATE::NEQ: return new Bool(!(*(Obj *) a == *(Obj *) b));
        case RELATION_PREDICATE::GT: {
          switch (a->type()) {
            case OType::INT: return new Bool(a->as<Int>()->value() > b->as<Int>()->value());
            case OType::REAL: return new Bool(a->as<Real>()->value() > b->as<Real>()->value());
            case OType::STR: return new Bool(a->as<Str>()->value() > b->as<Str>()->value());
            default: throw fError("Unable to relate %s by %s\n", OTYPE_STR.at(a->type()), REL_TO_STR(rel));
          }
        }
        case RELATION_PREDICATE::GTE: {
          switch (a->type()) {
            case OType::INT: return new Bool(a->as<Int>()->value() >= b->as<Int>()->value());
            case OType::REAL: return new Bool(a->as<Real>()->value() >= b->as<Real>()->value());
            case OType::STR: return new Bool(a->as<Str>()->value() >= b->as<Str>()->value());
            default: throw fError("Unable to relate %s by %s\n", OTYPE_STR.at(a->type()), REL_TO_STR(rel));
          }
        }
        case RELATION_PREDICATE::LT: {
          switch (a->type()) {
            case OType::INT: return new Bool(a->as<Int>()->value() < b->as<Int>()->value());
            case OType::REAL: return new Bool(a->as<Real>()->value() < b->as<Real>()->value());
            case OType::STR: return new Bool(a->as<Str>()->value() < b->as<Str>()->value());
            default: throw fError("Unable to relate %s by %s\n", OTYPE_STR.at(a->type()), REL_TO_STR(rel));
          }
        }
        case RELATION_PREDICATE::LTE: {
          switch (a->type()) {
            case OType::INT: return new Bool(a->as<Int>()->value() <= b->as<Int>()->value());
            case OType::REAL: return new Bool(a->as<Real>()->value() <= b->as<Real>()->value());
            case OType::STR: return new Bool(a->as<Str>()->value() <= b->as<Str>()->value());
            default: throw fError("Unable to relate %s by %s\n", OTYPE_STR.at(a->type()), REL_TO_STR(rel));
          }
        }
        default: {
          throw fError("Algebra doesn't define %s + %s", OTYPE_STR.at(a->type()),
                       OTYPE_STR.at(b->type()));
        }
      }
    }

    /////////////////////////////////////////////////////////////////////
    //////////////////////////// COMPOSITION ////////////////////////////
    /////////////////////////////////////////////////////////////////////

    enum class COMPOSITION_OPERATOR {
      PLUS,
      MULT,
      MINUS,
      DIV,
      NEG
    };

    static const char *COMP_TO_STR(const COMPOSITION_OPERATOR comp) {
      const static Map<const COMPOSITION_OPERATOR, const char *> map = {
        {COMPOSITION_OPERATOR::PLUS, "plus"},
        {COMPOSITION_OPERATOR::MULT, "mult"},
        {COMPOSITION_OPERATOR::MINUS, "minus"},
        {COMPOSITION_OPERATOR::DIV, "div"},
        {COMPOSITION_OPERATOR::NEG, "neg"},
      };
      return map.at(comp);
    }

    virtual const Obj *compose(const COMPOSITION_OPERATOR comp, const Obj *a, const Obj *b) const {
      switch (comp) {
        case COMPOSITION_OPERATOR::PLUS: {
          switch (a->type()) {
            case OType::URI: return new Uri(((Uri *) b)->value().extend(((Uri *) a)->toString().c_str()));
            case OType::BOOL: return new Bool(((Bool *) b)->value() || ((Bool *) a)->value());
            case OType::INT: return new Int(((Int *) b)->value() + ((Int *) a)->value());
            case OType::REAL: return new Real(((Real *) b)->value() + ((Real *) a)->value());
            case OType::STR: return new Str(
                string(((Str *) b)->value().c_str()).append(((Str *) a)->value()));
            default: {
              throw fError("Algebra doesn't define %s + %s", OTYPE_STR.at(a->type()),
                           OTYPE_STR.at(b->type()));
            }
          }
        }
        case COMPOSITION_OPERATOR::MULT: {
          switch (a->type()) {
            case OType::BOOL: return new Bool(((Bool *) b)->value() && ((Bool *) a)->value());
            case OType::INT: return new Int(((Int *) b)->value() * ((Int *) a)->value());
            case OType::REAL: return new Real(((Real *) b)->value() * ((Real *) a)->value());
            case OType::REC: {
             // RecMap<Obj *, Obj *> *map = new RecMap<Obj *, Obj *>();
              Rec *rec = new Rec({});
              auto ita = ((Rec *) a)->value()->begin();
              auto itb = ((Rec *) b)->value()->begin();
              for (; ita != ((Rec *) a)->value()->end(); ++ita) {
                rec->set(
                  (Obj *) compose(COMPOSITION_OPERATOR::MULT, ita->first, itb->first),
                  (Obj *) compose(COMPOSITION_OPERATOR::MULT, ita->second, itb->second));
                ++itb;
              }
              return rec;
            }
            default: {
              throw fError("Algebra doesn't define %s * %s", OTYPE_STR.at(a->type()),
                           OTYPE_STR.at(b->type()));
            }
          }
        }
        default: {
          throw fError("Algebra does not support composition %s on %s", COMP_TO_STR(comp),
                       OTYPE_STR.at(a->type()),
                       OTYPE_STR.at(b->type()));
        }
      }
    }
  };
} // namespace fhatos

#endif
