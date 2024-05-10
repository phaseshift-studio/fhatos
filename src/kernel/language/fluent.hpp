#ifndef fhatos_kernel__fluent_hpp
#define fhatos_kernel__fluent_hpp

#include <fhatos.hpp>
//
#include <kernel/language/obj.hpp>

namespace fhatos::kernel {

template <typename S, typename E> using Bytecode = List<Inst<S, E>>;

template <typename S, typename E> class Fluent {

  //////////////////////////////////////////////////////////////////////////////
  /////////////////////////    PUBLIC   ////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
public:
  Fluent(const S s)
      : bcode({Inst<S, S>(
            {"start", {new S(s)}, [s](const S &in) { return s; }})}) {}

  Fluent<S, E> plus(const E e) {
    return *this->addInst<E>("plus", {new E(e)}, [](const E &e) { return e; });
  }

  /////////////////////////////  TO_STRING /////////////////////////////////////
  const String toString() const {
    String s;
    int counter = 0;
    for (const auto &inst : this->bcode) {
      s = s + inst.toString();
    }
    return s;
  }
  //////////////////////////////////////////////////////////////////////////////
  /////////////////////////    PRIVATE   ///////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
private:
  Bytecode<S, E> bcode;
  Fluent<S, E>(Bytecode<S, E> bcode) : bcode(bcode) {}
  template <typename E2>
  Fluent<S, E2> *addInst(const char *op, const List<void *> &args,
                         const Function<E, E2> &function) {
    bcode.push_back(Inst<E, E2>({String(op), args, function}));
    return (Fluent<S, E2> *)this;
  }
};

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////    STATIC HELPERS   ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

template <typename S> const static Fluent<S, S> __(const S start) {
  return Fluent<S, S>(start);
}

} // namespace fhatos::kernel

#endif