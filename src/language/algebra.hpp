#ifndef fhatos_algebra_hpp
#define fhatos_algebra_hpp

#include <fhatos.hpp>
//
#include <language/obj.hpp>

namespace fhatos {
  //template<typename OBJ>
  //struct RingAlgebra : public Algebra<OBJ, RING> {
   // static Function<OBJ *, OBJ *> plus(OBJ *a, OBJ *b) {
  //    return [a](OBJ *b) { return new OBJ(a->value() + b->value()); };
   // }

    /*static const Function<OBJ, OBJ> minus(const OBJ &a, const OBJ &b) {
      return [a](const OBJ &b) { return OBJ(a.get() - b.get()); };
    }

    static const Function<OBJ, OBJ> mult(const OBJ &a, const OBJ &b) {
      return [a](const OBJ &b) { return OBJ(a.get() * b.get()); };
    }

    static const Supplier<OBJ> neg(const OBJ &a) {
      return [a]() { return OBJ(-a.get()); };
    }*/
  //};
} // namespace fhatos

#endif
