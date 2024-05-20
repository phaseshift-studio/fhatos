#ifndef fhatos_instructions_hpp
#define fhatos_instructions_hpp

#include <fhatos.hpp>
//
#include <any>
#include <language/algebra.hpp>
#include <language/obj.hpp>

namespace fhatos {
  template<typename S>
  class StartInst final : public Inst<S, S> {
  public:
    explicit StartInst(const List<S> starts)
      : Inst<S, S>({
        "start", ptr_list<S, Obj>(starts),
        [starts](S *b) -> S *{
          return new S(starts.front());
        }
      }) {
    }
  };

  template<typename E>
  class PlusInst final : public Inst<E, E> {
  public:
    explicit PlusInst(const S_E<Obj, E> *a)
      : Inst<E, E>({
        "plus", {Inst<E, E>::template convert<E>(a)->obj()},
        [this](const E *b) { return new E(this->template arg<S_E<Obj, E> >(0)->apply(b)->value() + b->value()); }
      }) {
    }

    const string toString() const override {
      return Inst<E, E>::makeString(this->opcode(), this->template arg<S_E<Obj, E> >(0)->toString());
    }
  };

  template<typename E>
  class MultInst final : public Inst<E, E> {
  public:
    explicit MultInst(const S_E<Obj, E> *a)
      : Inst<E, E>({
        "mult", {Inst<E, E>::template convert<E>(a)->obj()},
        [this](const E *b) { return new E(this->template arg<S_E<Obj, E> >(0)->apply(b)->value() * b->value()); }
      }) {
    }

    const string toString() const override {
      return Inst<E, E>::makeString(this->opcode(), this->template arg<S_E<Obj, E> >(0)->toString());
    }
  };
} // namespace fhatos

#endif
