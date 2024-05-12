#ifndef fhatos_instructions_hpp
#define fhatos_instructions_hpp

#include <fhatos.hpp>
//
#include <language/algebra.hpp>
#include <language/obj.hpp>

namespace fhatos {

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

} // namespace fhatos

#endif