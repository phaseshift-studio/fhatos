#ifndef fhatos_processor_hpp
#define fhatos_processor_hpp

#include <fhatos.hpp>
//
#include <language/obj.hpp>

namespace fhatos {
  template<typename A>
  class Monad {
  protected:
    A *_value;
    // const Inst<Obj, A> *inst = nullptr;
    const long _bulk = 1;

  public:
    template<typename X>
    using ptr = std::shared_ptr<Monad<X> >;

    explicit Monad(const Obj *value) : _value((A *) value) {
    }

    template<typename B>
    const Monad<B> *split(const Inst *next) const {
      return new Monad<B>((B *) next->apply(this->get()));
    }

    A *get() const { return this->_value; }

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
    const Bytecode *bcode;
    List<const E *> output;
    bool done = false;

  public:
    explicit Processor(const Bytecode *bcode) : bcode(bcode) {
    }

    void forEach(const Consumer<const E *> &consumer) {
      for (const E *end: this->toList()) {
        if (!((Obj *) end)->isNoObj())
          consumer(end);
      }
    }

    List<const E *> toList() {
      if (this->done)
        return this->output;
      this->done = true;
      LOG(DEBUG, "Processing bytecode: %s\n", this->bcode->toString().c_str());
      while (!this->bcode->startInst()->getOutput()->empty()) {
        const Obj *start = this->bcode->startInst()->getOutput()->back();
        this->bcode->startInst()->getOutput()->pop_back();
        if (!start)
          return this->output;
        LOG(DEBUG, FOS_TAB_2 "starting with %s [%s]\n", start->toString().c_str(), OTYPE_STR.at(start->type()).c_str());
        Monad<E> *end = new Monad<E>(start);
        //int counter = 0;
        for (auto it = ++this->bcode->value()->begin(); it != this->bcode->value()->end(); ++it) {
          // if (counter++ != 0) {
          LOG(DEBUG, FOS_TAB_3 "Processing: %s=>%s [M[%s]]\n", end->toString().c_str(), (**it).toString().c_str(),
              OTYPE_STR.at(end->get()->type()).c_str());
          end = const_cast<Monad<E> *>(end->template split<E>(*it));
          if (((Obj *) end->get())->isNoObj())
            break;
        }
        if (!((Obj *) end->get())->isNoObj())
          this->output.push_back(end->get());
      }
      return this->output;
    }

    bool hasNext() {
      return !this->toList().empty();
    }

    const E *next() {
      if (!this->hasNext())
        throw fError("No more obj results");
      const E *e = this->toList().front();
      this->toList().pop_front();
      return e;
    }
  };
} // namespace fhatos

#endif
