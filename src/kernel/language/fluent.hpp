#ifndef fhatos_kernel__fluent_hpp
#define fhatos_kernel__fluent_hpp

#include <fhatos.hpp>
//
#include <kernel/language/algebra.hpp>
#include <kernel/language/instructions.hpp>
#include <kernel/language/obj.hpp>
#include <kernel/language/processor.hpp>

namespace fhatos::kernel {

// template <typename S, typename E> using Bytecode = List<Inst<S, E>>;

template <typename S, typename E,
          typename PROCESSOR = Processor<S, E, Monad<E>>>
class Fluent { //: public E {

  //////////////////////////////////////////////////////////////////////////////
  /////////////////////////    PUBLIC   ////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
public:
  Fluent() : bcode(Bytecode<S, E>()) {}
  Fluent(const List<S *> *starts)
      : bcode(StartInst<S>((List<void *> *)(void *)starts)) {};

  const Fluent<S, E> plus(const E &e) const {
    return this->addInst<E>(PlusInst<E>(e));
  }
  const Fluent<S, E> plus(const Fluent<E, E> &e) const {
    return this->addInst<E>(
        Inst<E, E>({"plus", List<void *>({(void *)new Bytecode<E, E>(e.bcode)}),
                    [](E *e) { return e; }}));
  }
    const String toString() const { return "f" + this->bcode.toString(); }

  //////////////////////////////////////////////////////////////////////////////
  const E * next() const {
    static PROCESSOR proc = PROCESSOR(this->bcode);
    return proc.next();
  }

  const List<E *> toList() const {
    static PROCESSOR proc = PROCESSOR(this->bcode);
    return proc.toList();
  }

  const void forEach(const Consumer<const E *> consumer) const {
    static PROCESSOR proc = PROCESSOR(this->bcode);
    proc.forEach(consumer);
  }

  //////////////////////////////////////////////////////////////////////////////
  /////////////////////////    PRIVATE ///////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
private:
  const Bytecode<S, E> bcode;
  Fluent(const Bytecode<S, E> &bcode) : bcode(bcode) {}

  template <typename E2>
  const Fluent<S, E2> addInst(const Inst<E, E2> &inst) const {
    return Fluent<S, E2>(bcode.addInst(inst));
  }
};
//////////////////////////////////////////////////////////////////////////////
//////////////////////    STATIC HELPERS   ///////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

template <typename S>
const static Fluent<S, S> __(std::initializer_list<S> starts) {
  return Fluent<S, S>(ptr_list(List<S>(starts)));
};

template <typename S> const static Fluent<S, S> __(const S &start) {
  return __<S>({start});
};

template <typename S> const static Fluent<S, S> __() { return Fluent<S, S>(); };

} // namespace fhatos::kernel

#endif