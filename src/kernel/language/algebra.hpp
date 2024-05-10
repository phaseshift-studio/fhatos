#ifndef fhatos_kernel__algebra_hpp
#define fhatos_kernel__algebra_hpp

#include <fhatos.hpp>
//
#include <kernel/language/obj.hpp>

namespace fhatos::kernel {

template <typename OBJ> struct RingAlgebra : public Algebra<OBJ, RING> {

  static const OBJ plus(const OBJ &a, const OBJ &b) { return a; }

  static const OBJ minus(const OBJ &a, const OBJ &b) { return a; }

  static const OBJ mult(const OBJ &a, const OBJ &b) { return a; }

  static const OBJ neg(const OBJ &a) { return a; }
};

struct BoolAlgebra : public RingAlgebra<Bool> {

  static const Bool plus(const Bool &a, const Bool &b) {
    return Bool(a.get() || b.get());
  }
  static const Bool minus(const Bool &a, const Bool &b) {
    return Bool(a.get() || BoolAlgebra::neg(b).get());
  }
  static const Bool mult(const Bool &a, const Bool &b) {
    return Bool(a.get() && b.get());
  }
  static const Bool neg(const Bool &a) { return Bool(!a.get()); }
};

} // namespace fhatos::kernel

#endif