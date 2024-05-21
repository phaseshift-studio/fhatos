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

    virtual const Obj *plus(const Obj *a, const Obj *b) {
      switch (a->type()) {
        case URI: return new Uri(((Uri *) a)->value().extend(((Uri *) b)->toString().c_str()));
        case BOOL: return new Bool(((Bool *) a)->value() || ((Bool *) b)->value());
        case INT: return new Int(((Int *) a)->value() + ((Int *) b)->value());
        case REAL: return new Real(((Real *) a)->value() + ((Real *) b)->value());
        case STR: return new Str(string(((Str *) a)->value().c_str()).append(((Str *) b)->value()));
        default: {
          throw new fError("Algebra doesn't define %s + %s", OTYPE_STR.at(a->type()).c_str(),
                           OTYPE_STR.at(b->type()).c_str());
          return nullptr;
        }
      }
    }

    virtual const Obj *mult(const Obj *a, const Obj *b) {
      switch (a->type()) {
        case BOOL: return new Bool(((Bool *) a)->value() && ((Bool *) b)->value());
        case INT: return new Int(((Int *) a)->value() * ((Int *) b)->value());
        case REAL: return new Real(((Real *) a)->value() * ((Real *) b)->value());
        default: {
          throw new fError("Algebra doesn't define %s * %s", OTYPE_STR.at(a->type()).c_str(),
                           OTYPE_STR.at(b->type()).c_str());
          return nullptr;
        }
      }
    }
  };

  //};
} // namespace fhatos

#endif
