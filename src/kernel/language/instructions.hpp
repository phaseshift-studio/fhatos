#ifndef fhatos_kernel__instructions_hpp
#define fhatos_kernel__instructions_hpp

#include <fhatos.hpp>
//
#include <kernel/language/algebra.hpp>
#include <kernel/language/obj.hpp>

namespace fhatos::kernel {

template <typename A> class StartInst : public Inst<A, A> {
public:
  StartInst(const List<void*>* starts)
      : Inst<A, A>({"start", *starts,
                    [starts](A *b) { return (A*)starts->front(); }}) {};
};

template <typename A> class PlusInst : public Inst<A, A> {
public:
  PlusInst(const A &a)
      : Inst<A, A>(
            {"plus", {new A(a)}, RingAlgebra<A>::plus(new A(a), new A(a))}) {};
};

} // namespace fhatos::kernel

#endif