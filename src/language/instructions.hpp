#ifndef fhatos_instructions_hpp
#define fhatos_instructions_hpp

#include <fhatos.hpp>
//
#include <language/algebra.hpp>
#include <language/obj.hpp>

namespace fhatos {
  template<typename S>
  class StartInst final : public Inst<S, S> {
  public:
    explicit StartInst(const List<S> starts)
      : Inst<S, S>({
        "start", ptr_list<S, Obj>(starts),
        [starts](S b) {
          return starts.front();
        }
      }) {
    }
  };

  template<typename E>
  class PlusInst final : public Inst<E, E> {
  public:
    explicit PlusInst(const E &a)
      : Inst<E, E>({"plus", ptr_list<E, Obj>({a}), [a](E b) { return E(a.value() + b.value()); }}) {
    }
  };
} // namespace fhatos

#endif
