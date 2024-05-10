#ifndef fhatos_kernel__bytecode_hpp
#define fhatos_kernel__bytecode_hpp

#include <fhatos.hpp>
//
#include <kernel/language/algebra.hpp>
#include <kernel/language/obj.hpp>

namespace fhatos::kernel {

template <typename S, typename E> using Bytecode2 = List<Inst<S, E>>;

template <typename S, typename E> class Bytecode2 : Inst<S, E> {

  List<Inst<ObjX, ObjX>> _insts;

  Bytecode2(const Bytecode2<S, E> copy) : Inst(copy.args()) {}
  Bytecode2(const List<void *> insts) : _insts((List<Inst<ObjX, ObjX>>)insts) {}
  virtual const String opcode() const override { return "bc"; }
};
} // namespace fhatos::kernel
#endif