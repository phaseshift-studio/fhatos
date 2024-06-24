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
#include <util/obj_helper.hpp>

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

    /*enum class BRANCH_SEMANTIC { SPLIT, CHAIN, SWITCH };

    static const char *BRNCH_TO_STR(const BRANCH_SEMANTIC brnch) {
      const static Map<const BRANCH_SEMANTIC, const char *> map = {
          {BRANCH_SEMANTIC::SPLIT, "split"}, {BRANCH_SEMANTIC::CHAIN, "chain"}, {BRANCH_SEMANTIC::SWITCH, "switch"}};
      return map.at(brnch);
    }

    virtual const ptr<Obj> branch(const BRANCH_SEMANTIC branch, const ptr<Obj> incoming, const ptr<Obj> control) const {
      switch (branch) {
        case BRANCH_SEMANTIC::SPLIT: {
          return Obj::to_noobj();
        }
        case BRANCH_SEMANTIC::CHAIN: {
          return Obj::to_noobj();
        }
        case BRANCH_SEMANTIC::SWITCH: {
          for (const auto &[check, outgoing]: ((Rec *) control.get())->value()) {
            if (!check->apply(incoming)->isNoObj()) {
              return outgoing->apply(incoming);
            }
          }
          return Obj::to_noobj();;
        }
        default: {
          throw fError("Algebra doesn't define %s + %s", OTYPE_STR.at(incoming->o_range()),
                       OTYPE_STR.at(control->o_domain()));
        }
      }
    }

    ///////////////////////////////////////////////////////////////////
    //////////////////////////// RELATIONS ////////////////////////////
    ///////////////////////////////////////////////////////////////////

    enum class RELATION_PREDICATE { EQ, NEQ, GT, GTE, LT, LTE };

    static const char *REL_TO_STR(const RELATION_PREDICATE rel) {
      const static Map<const RELATION_PREDICATE, const char *> map = {
          {RELATION_PREDICATE::EQ, "eq"},   {RELATION_PREDICATE::NEQ, "neq"}, {RELATION_PREDICATE::GT, "gt"},
          {RELATION_PREDICATE::GTE, "gte"}, {RELATION_PREDICATE::LT, "lt"},   {RELATION_PREDICATE::LTE, "lte"}};
      return map.at(rel);
    }

    virtual const ptr<Obj> relate(const RELATION_PREDICATE rel, const ptr<Obj> &lhs, const ptr<Obj> &rhs) {
      if (const fError *e = ObjHelper::sameTypes(lhs, rhs)) {
        throw *e;
      }
      switch (rel) {
        case RELATION_PREDICATE::EQ:
          return ptr<Obj>(new Bool(*lhs == *rhs));
        case RELATION_PREDICATE::NEQ:
          return ptr<Obj>(new Bool(*lhs != *rhs));
        case RELATION_PREDICATE::GT: {
          switch (lhs->otype()) {
            case OType::INT: {
              auto castLHS = std::static_pointer_cast<Int>(lhs);
              auto castRHS = std::static_pointer_cast<Int>(rhs);
              return share<Bool>(Bool(castLHS->value() > castRHS->value()));
            }
            case OType::REAL:
              return ptr<Obj>(new Bool(((Real *) lhs.get())->value() > ((Real *) rhs.get())->value()));
            case OType::STR:
              return ptr<Obj>(new Bool(((Str *) lhs.get())->value() > ((Str *) rhs.get())->value()));
            default:
              throw fError("Unable to relate %s by %s\n", OTYPE_STR.at(lhs->otype()), REL_TO_STR(rel));
          }
        }
        case RELATION_PREDICATE::GTE: {
          switch (lhs->otype()) {
            case OType::INT: {
              auto castLHS = std::static_pointer_cast<Int>(lhs);
              auto castRHS = std::static_pointer_cast<Int>(rhs);
              return share<Bool>(Bool(castLHS->value() >= castRHS->value()));
            }
            case OType::REAL:
              return ptr<Obj>(new Bool(((Real *) lhs.get())->value() >= ((Real *) rhs.get())->value()));
            case OType::STR:
              return ptr<Obj>(new Bool(((Str *) lhs.get())->value() >= ((Str *) rhs.get())->value()));
            default:
              throw fError("Unable to relate %s by %s\n", OTYPE_STR.at(lhs->otype()), REL_TO_STR(rel));
          }
        }
        case RELATION_PREDICATE::LT: {
          switch (lhs->otype()) {
            case OType::INT: {
              auto castLHS = std::static_pointer_cast<Int>(lhs);
              auto castRHS = std::static_pointer_cast<Int>(rhs);
              return share<Bool>(Bool(castLHS->value() < castRHS->value()));
            }
            case OType::REAL:
              return ptr<Obj>(new Bool(((Real *) lhs.get())->value() < ((Real *) rhs.get())->value()));
            case OType::STR:
              return ptr<Obj>(new Bool(((Str *) lhs.get())->value() < ((Str *) rhs.get())->value()));
            default:
              throw fError("Unable to relate %s by %s\n", OTYPE_STR.at(lhs->otype()), REL_TO_STR(rel));
          }
        }
        case RELATION_PREDICATE::LTE: {
          switch (lhs->otype()) {
            case OType::INT: {
              auto castLHS = std::static_pointer_cast<Int>(lhs);
              auto castRHS = std::static_pointer_cast<Int>(rhs);
              return share<Bool>(Bool(castLHS->value() <= castRHS->value()));
            }
            case OType::REAL:
              return ptr<Obj>(new Bool(((Real *) lhs.get())->value() <= ((Real *) rhs.get())->value()));
            case OType::STR:
              return ptr<Obj>(new Bool(((Str *) lhs.get())->value() <= ((Str *) rhs.get())->value()));
            default:
              throw fError("Unable to relate %s by %s\n", OTYPE_STR.at(lhs->otype()), REL_TO_STR(rel));
          }
        }
        default: {
          throw fError("Algebra doesn't define %s + %s", OTYPE_STR.at(lhs->otype()), OTYPE_STR.at(rhs->otype()));
        }
      }
    }

    /////////////////////////////////////////////////////////////////////
    //////////////////////////// COMPOSITION ////////////////////////////
    /////////////////////////////////////////////////////////////////////

    enum class COMPOSITION_OPERATOR { PLUS, MULT, MINUS, DIV, NEG, MOD };

    static const char *COMP_TO_STR(const COMPOSITION_OPERATOR comp) {
      const static Map<const COMPOSITION_OPERATOR, const char *> map = {
          {COMPOSITION_OPERATOR::PLUS, "plus"},   {COMPOSITION_OPERATOR::MULT, "mult"},
          {COMPOSITION_OPERATOR::MINUS, "minus"}, {COMPOSITION_OPERATOR::DIV, "div"},
          {COMPOSITION_OPERATOR::NEG, "neg"},     {COMPOSITION_OPERATOR::MOD, "mod"}};
      return map.at(comp);
    }

    virtual const ptr<Obj> compose(const COMPOSITION_OPERATOR &comp, const ptr<Obj> &lhs, const ptr<Obj> &rhs) const {
      if (const fError *e = ObjHelper::sameTypes(lhs, rhs)) {
        throw *e;
      }
      switch (comp) {
        case COMPOSITION_OPERATOR::PLUS: {
          switch (lhs->otype()) {
            case OType::BOOL:
              return ((Bool *) lhs.get())->split(((Bool *) lhs.get())->value() || ((Bool *) rhs.get())->value());
            case OType::INT: {
              auto castLHS = std::static_pointer_cast<Int>(lhs);
              auto castRHS = std::static_pointer_cast<Int>(rhs);
              return share<Int>(*castLHS + *castRHS);
            }
            case OType::REAL: {
              auto castLHS = std::static_pointer_cast<Real>(lhs);
              auto castRHS = std::static_pointer_cast<Real>(rhs);
              return castLHS->split(castLHS->value() + castRHS->value());
            }
            case OType::URI: {
              auto castLHS = std::static_pointer_cast<Uri>(lhs);
              auto castRHS = std::static_pointer_cast<Uri>(rhs);
              return castLHS->split(castLHS->value().extend(castRHS->value().toString().c_str()));
            }
            case OType::STR: {
              auto castLHS = std::static_pointer_cast<Str>(lhs);
              auto castRHS = std::static_pointer_cast<Str>(rhs);
              return castLHS->split(castLHS->value().append(castRHS->value()));
            }
            default: {
              throw fError("Algebra doesn't define %s + %s", OTYPE_STR.at(lhs->otype()), OTYPE_STR.at(rhs->otype()));
            }
          }
        }
        case COMPOSITION_OPERATOR::MULT: {
          switch (lhs->otype()) {
            case OType::BOOL: {
              ptr<Bool> castLHS = std::static_pointer_cast<Bool>(lhs);
              ptr<Bool> castRHS = std::static_pointer_cast<Bool>(rhs);
              return castLHS->split(castLHS->value() && castRHS->value());
            }
            case OType::INT: {
              auto castLHS = std::static_pointer_cast<Int>(lhs);
              auto castRHS = std::static_pointer_cast<Int>(rhs);
              return share<Int>(*castLHS * *castRHS);
            }
            case OType::REAL: {
              auto castLHS = std::static_pointer_cast<Real>(lhs);
              auto castRHS = std::static_pointer_cast<Real>(rhs);
              return castLHS->split(castLHS->value() * castRHS->value());
            }
            case OType::REC: {
              // RecMap<Obj *, Obj *> *map = new RecMap<Obj *, Obj *>();
              Rec *rec = new Rec({});
              auto itLHS = ((Rec *) lhs.get())->value().begin();
              auto itRHS = ((Rec *) rhs.get())->value().begin();
              for (; itLHS != ((Rec *) lhs.get())->value().end(); ++itLHS) {
                ptr<Obj> k = compose(COMPOSITION_OPERATOR::MULT, itLHS->first, itRHS->first);
                ptr<Obj> v = compose(COMPOSITION_OPERATOR::MULT, itLHS->second, itRHS->second);
                rec->set(k, v);
                ++itRHS;
              }
              return ptr<Rec>(rec);
            }
            default: {
              throw fError("Algebra doesn't define %s * %s", OTYPE_STR.at(lhs->otype()), OTYPE_STR.at(rhs->otype()));
            }
          }
          case COMPOSITION_OPERATOR::MOD: {
            switch (lhs->otype()) {
              case OType::INT:
                return ((Int *) lhs.get())->split(((Int *) lhs.get())->value() % ((Int *) rhs.get())->value());
              default: {
                throw fError("Algebra doesn't define %s %% %s", OTYPE_STR.at(lhs->otype()), OTYPE_STR.at(rhs->otype()));
              }
            }
          }
        }
        default: {
          throw fError("Algebra does not support composition %s on %s", COMP_TO_STR(comp), OTYPE_STR.at(lhs->otype()),
                       OTYPE_STR.at(rhs->otype()));
        }
      }
    }
  };*/
  };
} // namespace fhatos

#endif
