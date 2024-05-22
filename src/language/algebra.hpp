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
        case URI: return new Uri(((Uri *) b)->value().extend(((Uri *) a)->toString().c_str()));
        case BOOL: return new Bool(((Bool *) b)->value() || ((Bool *) a)->value());
        case INT: return new Int(((Int *) b)->value() + ((Int *) a)->value());
        case REAL: return new Real(((Real *) b)->value() + ((Real *) a)->value());
        case STR: return new Str(string(((Str *) b)->value().c_str()).append(((Str *) a)->value()));
        default: {
          throw  fError("Algebra doesn't define %s + %s", OTYPE_STR.at(a->type()).c_str(),
                       OTYPE_STR.at(b->type()).c_str());
        }
      }
    }

    virtual const Obj *mult(const Obj *a, const Obj *b) {
      switch (a->type()) {
        case BOOL: return new Bool(((Bool *) b)->value() && ((Bool *) a)->value());
        case INT: return new Int(((Int *) b)->value() * ((Int *) a)->value());
        case REAL: return new Real(((Real *) b)->value() * ((Real *) a)->value());
        default: {
          throw  fError("Algebra doesn't define %s * %s", OTYPE_STR.at(a->type()).c_str(),
                       OTYPE_STR.at(b->type()).c_str());
        }
      }
    }
  };

  //};
} // namespace fhatos

#endif
