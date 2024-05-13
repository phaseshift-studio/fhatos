#ifndef fhatos_instructions_hpp
#define fhatos_instructions_hpp

#include <fhatos.hpp>
//
#include <language/algebra.hpp>
#include <language/obj.hpp>

namespace fhatos {
  template<typename A>
  class StartInst final : public Inst<A, A> {
  public:
    explicit StartInst(const List<void *> *starts)
      : Inst<A, A>({
        "start", *starts,
        [starts](A *b) { return static_cast<A *>(starts->front()); }
      }) {
    }
  };

  template<typename A>
  class PlusInst final : public Inst<A, A> {
  public:
    explicit PlusInst(const A &a)
      : Inst<A, A>(
        {"plus", {new A(a)}, RingAlgebra<A>::plus(new A(a), new A(a))}) {
    }
  };
} // namespace fhatos

#endif
