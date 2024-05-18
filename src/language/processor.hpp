#ifndef fhatos_processor_hpp
#define fhatos_processor_hpp

#include <fhatos.hpp>
//
#include <language/obj.hpp>

namespace fhatos {
  template<typename A>
  class Monad {
  protected:
    const A *value;
    const Inst<Obj, A> *inst = nullptr;
    const long _bulk = 1;

  public:
    explicit Monad(const A *value) : value(value) {
    }

    template<typename B>
    Monad<B> *split(Inst<A, B> *next) const {
      return new Monad<B>(new B(next->apply(*this->get())));
    }

    const A *get() const { return this->value; }

    long bulk() const { return this->_bulk; }

    const Inst<Obj, A> *at() const { return this->inst; }
    // const bool equals(const Monad<ObjX> &other) const {
    //   return this->value.equals(other.get());
    // }
  };

  template<typename S, typename E, typename MONAD = Monad<Obj> >
  class Processor {
  protected:
    const Bytecode<S, E> bcode;
    List<E *> ends;

  public:
    explicit Processor(const Bytecode<S, E> &bcode) : bcode(bcode) {
    }

    void forEach(const Consumer<const E *> &consumer) {
      for (const auto *end: this->toList()) {
        consumer(end);
      }
    }

    List<E *> toList() {
      static bool done = false;
      if (done)
        return this->ends;
      done = true;
      const auto starts = List<Obj *>(this->bcode.value().front().args());
      for (const auto *start: starts) {
        Monad<E> *end = new Monad<E>((E*)start);
        int counter = 0;
        for (const auto inst: this->bcode.value()) {
          if (counter++ != 0)
            end = end->split((Inst<E, E> *) (&inst));
        }
        this->ends.push_back((E*)(void*)end->get());
      }
      return this->ends;
    }

    bool hasNext() {
      return !this->toList().empty();
    }

    const E *next() {
      if (!this->hasNext())
        throw std::runtime_error("No more obj results");
      const E *e = this->toList().front();
      this->toList().pop_front();
      return e;
    }
  };
} // namespace fhatos

#endif
