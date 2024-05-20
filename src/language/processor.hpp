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
    // const Inst<Obj, A> *inst = nullptr;
    const long _bulk = 1;

  public:
    explicit Monad(const A *value) : value(value) {
    }

    template<typename B>
    std::shared_ptr<Monad<B> > split(const Inst<A, B> *next) const {
      return std::make_shared<Monad<B> >(Monad<B>(next->apply(this->get())));
    }

    const A* get() const { return (A*) this->value; }

    long bulk() const { return this->_bulk; }

    const string toString() const {
      return string("M[") + this->get()->toString() + "]";
    }

    // const Inst<Obj, A> *at() const { return this->inst; }
    // const bool equals(const Monad<ObjX> &other) const {
    //   return this->value.equals(other.get());
    // }
  };

  template<typename S, typename E, typename MONAD = Monad<Obj> >
  class Processor {
  protected:
    const Bytecode<S, E> *bcode;
    List<E *> ends;
    bool done = false;

  public:
    explicit Processor(const Bytecode<S, E> *bcode) : bcode(bcode) {
    }

    void forEach(const Consumer<const E *> &consumer) {
      for (const E *end: this->toList()) {
        consumer(end);
      }
    }

    List<E *> toList() {
      if (this->done)
        return this->ends;
      this->done = true;
      LOG(DEBUG, "Processing bytecode: %s\n", this->bcode->toString().c_str());
      const auto starts = List<Obj *>(this->bcode->value().front()->args());
      for (const auto *start: starts) {
        LOG(DEBUG, FOS_TAB_2 "starting with %s\n", start->toString().c_str());
        std::shared_ptr<Monad<Obj> > end = std::make_shared<Monad<Obj> >(start);
        int counter = 0;
        for (const auto* inst: this->bcode->value()) {
          if (counter++ != 0) {
            LOG(DEBUG, FOS_TAB_3 "Processing: %s=>%s\n", end->toString().c_str(), inst->toString().c_str());
            end = end->split((Inst<Obj, Obj>*) inst);
          }
        }
        this->ends.push_back((E*)end->get());
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
