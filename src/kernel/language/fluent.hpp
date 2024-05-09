#ifndef fhatos_kernel__fluent_hpp
#define fhatos_kernel__fluent_hpp

#include <fhatos.hpp>
//
#include <kernel/language/obj.hpp>

namespace fhatos::kernel {

template <typename S, typename E> using Bytecode = List<Inst<S, E>>;

template <typename S, typename E> class Fluent {
protected:
  Bytecode<S, E> bcode;

public:
  Fluent(const S s) : bcode(Bytecode<S, S>()) {
    this->bcode.push_back(
        Inst<S, S>({Lst(List<ObjX>{new Str("start"), new S(s)}),
                    [this](const S &s) { return s; }}));
  };

  Fluent<S, E>(Bytecode<S, E> bcode) : bcode(bcode) {}

  const Fluent<S, E> plus(const E e) const {
    Bytecode<S, E> newBytecode = Bytecode<S, E>((Bytecode<S, E>)this->bcode);
    newBytecode.push_back(
        Inst<E, E>({Lst(List<ObjX>{new Str("plus"), new E(e)}),
                    [this](const E &e) { return e.plus(e); }}));
    return Fluent<S, E>(newBytecode);
  }

  const String toString() const {
    String s = ((Lst)this->bcode.front().get().first).toStr() + "=>";
    int counter = 0;
    for (const auto &inst : this->bcode) {
      if (counter++ > 0)
        s = s + inst.toStr();
    }
    return s;
  }
};

} // namespace fhatos::kernel

#endif