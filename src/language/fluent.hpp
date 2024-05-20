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
  class Fluent : public S_E<Obj, E> {
    //////////////////////////////////////////////////////////////////////////////
    /////////////////////////    PUBLIC   ////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
  public:
    explicit Fluent() : bcode(new Bytecode<S, E>()) {
    }

    explicit Fluent(const Bytecode<S, E> *bcode) : bcode(bcode) {
    }

    //////////////////////////////////////////////////////////////////////////////
    const E *apply(const Obj *start) const override {
      return this->bcode->apply(start);
    }

    const Bytecode<S, E> *self() const override {
      return this->bcode->self();
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
      PROCESSOR proc = PROCESSOR(this->bcode);
      proc.forEach(consumer);
    }

    const string toString() const override { return "f" + this->bcode->toString(); }

    //////////////////////////////////////////////////////////////////////////////
    ///////////////////////// PROTECTED  /////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
  protected:
    const Bytecode<S, E> *bcode;

    template<typename E2>
    Fluent<S, E2> addInst(const Inst<E, E2> *inst) const {
      return Fluent<S, E2>(this->bcode->addInst(inst));
    }

  public:
    //////////////////////////////////////////////////////////////////////////////
    ///////////////////////// INSTRUCTIONS ///////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////

    Fluent<S, E> plus(const S_E<Obj, E> &e) const {
      return this->template addInst<E>(new PlusInst<E>(&e));
    }

    Fluent<S, E> mult(const S_E<Obj, E> &e) const {
      return this->template addInst<E>(new MultInst<E>(&e));
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////    STATIC HELPERS   ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  template<typename S>
  inline static Fluent<S, S> __(const List<S> &starts) {
    return Fluent<S, S>(new Bytecode<S, S>(List<Inst<S, S> *>({new StartInst<S>(starts)})));
  };

  template<typename S>
  inline static Fluent<S, S> __(const S &start) {
    return Fluent<S, S>(new Bytecode<S, S>(List<Inst<S, S> *>({new StartInst<S>({start})})));
  };

  template<typename S>
  inline static Fluent<S, S> __() { return Fluent<S, S>(); };
} // namespace fhatos

#endif
