#ifndef fhatos_instructions_hpp
#define fhatos_instructions_hpp

#include <fhatos.hpp>
//
#include <any>
#include <language/algebra.hpp>
#include <language/obj.hpp>

namespace fhatos {
  class S_E;

  template <typename S>
  static List<Obj*>* cast(const List<S*>* list) {
    List<Obj*>* newList = new List<Obj*>();
    for(const auto s : *list) {
      newList->push_back((Obj*)s);
    }
    return newList;
  }

  template<typename S>
  class StartInst final : public Inst<S, S> {
  public:
    explicit StartInst(const List<S*>* starts)
      : Inst<S, S>({
        "start", *cast(starts),
        [starts](S *b) -> S *{
          return new S(((S*)starts->front())->value());
        }
      }) {
    }
  };

  template<typename E>
  class PlusInst final : public Inst<E, E> {
  public:
    explicit PlusInst(const E *a)
      : Inst<E, E>({
        "plus", List<Obj*>({(Obj*)a->obj()}),
        [this](const E *b) { return new E(this->template arg<E>(0)->apply(b)->value() + b->value()); }
      }) {
    }

    virtual const string toString() const override {
      return Inst<E, E>::makeString(this->opcode(), ((E*)this->template arg<E>(0))->toString());
    }
  };

  template<typename E>
  class MultInst final : public Inst<E, E> {
  public:
    explicit MultInst(const E *a)
      : Inst<E, E>({
        "mult", List<Obj*>({(Obj*)a->obj()}),
        [this](const E *b) { return new E(this->template arg<E>(0)->apply(b)->value() * b->value()); }
      }) {
    }

    const string toString() const override {
      return Inst<E, E>::makeString(this->opcode(), ((E*)this->template arg<E>(0))->toString());
    }
  };
} // namespace fhatos

#endif
