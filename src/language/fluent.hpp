#ifndef fhatos_fluent_hpp
#define fhatos_fluent_hpp

#include <fhatos.hpp>
//
#include <language/instructions.hpp>
#include <language/obj.hpp>
#include <language/processor.hpp>

namespace fhatos {
  template<typename S, typename E,
    typename PROCESSOR = Processor<S, E, Monad<E> > >
  class Fluent : public S_E<S, E> {
    //: public E {

    //////////////////////////////////////////////////////////////////////////////
    /////////////////////////    PUBLIC   ////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
  public:
    Fluent() : bcode(Bytecode<S, E>()) {
    }

    explicit Fluent(const List<S> starts)
      : bcode({StartInst<S>(starts)}) {
    };

    Fluent<S, E> plus(const E &e) const {
      return this->template addInst<E>(PlusInst<E>(e));
    }

    Fluent<S, E> plus(const Fluent<E, E> &e) const {
      return this->addInst<E>(
        Inst<E, E>({
          "plus", List<Obj *>({new Bytecode<E, E>(e.bcode)}),
          [](S e2) { return e2; }
        }));
    }

    string toString() const { return "f" + this->bcode.toString(); }

    //////////////////////////////////////////////////////////////////////////////
    virtual const E apply(const S start) const {
      return this->bcode.apply(start);
    }

    const E *next() const {
      static PROCESSOR proc = PROCESSOR(this->bcode);
      return proc.next();
    }

    const List<E *> &toList() const {
      static PROCESSOR proc = PROCESSOR(this->bcode);
      return proc.toList();
    }

    void forEach(const Consumer<const E *> &consumer) const {
      static PROCESSOR proc = PROCESSOR(this->bcode);
      proc.forEach(consumer);
    }

    //////////////////////////////////////////////////////////////////////////////
    /////////////////////////    PRIVATE ///////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
  private:
    const Bytecode<S, E> bcode;

    explicit Fluent(const Bytecode<S, E> &bcode) : bcode(bcode) {
    }

    template<typename E2>
    Fluent<S, E2> addInst(const Inst<E, E2> &inst) const {
      return Fluent<S, E2>(bcode.addInst(inst));
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////    STATIC HELPERS   ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  template<typename S>
  inline static Fluent<S, S> __(std::initializer_list<S> starts) {
    return Fluent<S, S>(List<S>(starts));
  };

  template<typename S>
  inline static Fluent<S, S> __(const S start) {
    return Fluent<S, S>(List<S>{start});
  };

  template<typename S>
  inline static Fluent<S, S> __() { return Fluent<S, S>(); };
} // namespace fhatos

#endif
