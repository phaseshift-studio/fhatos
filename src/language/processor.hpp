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
    const Inst<ObjY, A> *inst = nullptr;
    const long _bulk = 1;

  public:
    explicit Monad(const A *value) : value(value) {
    }

    template<typename B>
    const Monad<B> *split(const Inst<A, B> *next) const {
      return new Monad<B>(next->apply(this->get()));
    }

    const A *get() const { return this->value; }

    long bulk() const { return this->_bulk; }

    const Inst<ObjX, A> *at() const { return this->inst; }
    // const bool equals(const Monad<ObjX> &other) const {
    //   return this->value.equals(other.get());
    // }
  };

  template<typename S, typename E, typename MONAD = Monad<E> >
  class Processor {
  protected:
    const Bytecode<S, E> bcode;
    List<const E *> ends;

  public:
    explicit Processor(const Bytecode<S, E> &bcode) : bcode(bcode) {
    }

    void forEach(const Consumer<const E *> &consumer) {
      for (const auto *end: this->toList()) {
        consumer(end);
      }
    }

    List<const E *> toList() {
      static bool done = false;
      if (done)
        return this->ends;
      done = true;
      const auto starts = List<void *>(this->bcode.value().front().args());
      for (const auto *start: starts) {
        const MONAD *end = new MONAD(static_cast<const E *>(start));
        int counter = 0;
        for (const auto &inst: this->bcode.value()) {
          if (counter++ != 0)
            end = static_cast<const MONAD *>(static_cast<const void *>(end->split(&inst)));
        }
        this->ends.push_back(end->get());
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
