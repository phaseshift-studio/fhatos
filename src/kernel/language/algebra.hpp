#ifndef fhatos_kernel__algebra_hpp
#define fhatos_kernel__algebra_hpp

#include <fhatos.hpp>
//
#include <kernel/language/obj.hpp>

namespace fhatos::kernel {

template <typename OBJ> struct RingAlgebra : public Algebra<OBJ, RING> {

  static const Function<OBJ, OBJ> plus(const OBJ &a, const OBJ &b) {
    return [a](const OBJ &b) { return OBJ(a.get() + b.get()); };
  }

  static const Function<OBJ, OBJ> minus(const OBJ &a, const OBJ &b) {
    return [a](const OBJ &b) { return OBJ(a.get() - b.get()); };
  }

  static const Function<OBJ, OBJ> mult(const OBJ &a, const OBJ &b) {
    return [a](const OBJ &b) { return OBJ(a.get() * b.get()); };
  }

  static const Supplier<OBJ> neg(const OBJ &a) {
    return [a]() { return OBJ(-a.get()); };
  }
};

} // namespace fhatos::kernel

#endif