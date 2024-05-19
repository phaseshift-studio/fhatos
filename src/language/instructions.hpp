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
    explicit PlusInst(const E *a)
      : Inst<E, E>({
        "plus", {(Obj *) a},
        [this](const E *b) { return new E(this->template arg<E>(0)->apply(b)->value() + b->value()); }
      }) {
    }

    template<typename S>
    explicit PlusInst(const Bytecode<S, E> *a)
      : Inst<E, E>({
        "plus", {(Obj *) a},
        [this](const E *b) { return new E(this->template arg<Bytecode<S, E> >(0)->apply(b)->value() + b->value()); }
      }) {
    }
  };
} // namespace fhatos

#endif
