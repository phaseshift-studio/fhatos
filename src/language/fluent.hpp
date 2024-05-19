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
    explicit Fluent() : bcode(new Bytecode<S, E>()) {
    }

    explicit Fluent(Bytecode<S, E> *bcode) : bcode(bcode) {
    }

    Fluent<S, E> plus(const E &e) const {
      return this->template addInst<E>(PlusInst<E>(&e));
    }

    Fluent<S, E> plus(const Fluent<E, E> &e) const {
      return this->template addInst<E>(PlusInst<E>(e.bcode));
    }

    string toString() const { return "f" + this->bcode->toString(); }

    //////////////////////////////////////////////////////////////////////////////
    virtual const E *apply(const S *start) const {
      return this->bcode->apply(start);
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

    //////////////////////////////////////////////////////////////////////////////
    /////////////////////////    PRIVATE ///////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////
  private:
    const Bytecode<S, E> *bcode;

    template<typename E2>
    Fluent<S, E2> addInst(const Inst<E, E2> &inst) const {
      return Fluent<S, E2>(this->bcode->addInst(inst));
    }
  };

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////    STATIC HELPERS   ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////

  template<typename S>
  inline static Fluent<S, S> __(const List<S> &starts) {
    return Fluent<S, S>(new Bytecode<S, S>(List<Inst<S, S> >({StartInst<S>(starts)})));
  };

  template<typename S>
  inline static Fluent<S, S> __(const S &start) {
    return Fluent<S, S>(new Bytecode<S, S>(List<Inst<S, S> >({StartInst<S>({start})})));
  };

  template<typename S>
  inline static Fluent<S, S> __() { return Fluent<S, S>(); };
} // namespace fhatos

#endif
