#ifndef fhatos_kernel__processor_hpp
#define fhatos_kernel__processor_hpp

#include <fhatos.hpp>
//
#include <kernel/language/instructions.hpp>
#include <kernel/language/obj.hpp>

namespace fhatos::kernel {

template <typename S, typename E, typename MONAD = Monad<E>> class Processor {

protected:
  const Bytecode<S, E> bcode;

public:
  Processor(const Bytecode<S, E>& bcode) : bcode(bcode) {}

  List<E *> resultSet() { return List<E *>(); }

  const E *next() {
    MONAD *start = new MONAD((E *)this->bcode.get().front().args().front());
    int counter = 0;
    for (const auto &inst : this->bcode.get()) {
      if (counter++ != 0)
        start = (MONAD *)(void *)start->split(&inst);
    }
    return start->get();
  }
};

} // namespace fhatos::kernel

#endif