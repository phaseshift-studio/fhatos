#ifndef fhatos_kernel__fluent_hpp
#define fhatos_kernel__fluent_hpp

#include <fhatos.hpp>
//
#include <kernel/language/algebra.hpp>
#include <kernel/language/obj.hpp>

namespace fhatos::kernel {

// template <typename S, typename E> using Bytecode = List<Inst<S, E>>;

template <typename S, typename E> class Fluent {

  //////////////////////////////////////////////////////////////////////////////
  /////////////////////////    PUBLIC   ////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
public:
  Fluent() : bcode(Bytecode<S, E>()) {}
  Fluent(const S &s)
      : bcode({Inst<S, S>(
            {"start", {new S(s)}, [s](const S &in) { return s; }})}) {}

  const Fluent<S, E> plus(const E &e) const {
    return this->addInst<E>("plus", {new E(e)}, RingAlgebra<E>::plus(e, e));
  }
  const Fluent<S, E> plus(const Fluent<E, E> &e) const {
    return this->addInst<E>("plus", {new Bytecode<E, E>(e.bcode)},
                            [](const E &e) { return e; });
  }

  /////////////////////////////  TO_STRING
  ////////////////////////////////////////
  const String toString() const {
    String s;
    int counter = 0;
    for (auto &inst : this->bcode.get()) {
      s = s + inst.toString();
    }
    return s;
  }

  /*struct Iterator
      {
          using iterator_category = std::input_iterator_tag;
          using difference_type   = std::ptrdiff_t;
          using value_type        = E;
          using pointer           = E*;
          using reference         = E&;

          Iterator(pointer ptr) : m_ptr(ptr) {}

          reference operator*() const { return *e_ptr; }
          pointer operator->() { return e_ptr; }
          Iterator& operator++() { e_ptr++; return *this; }
          Iterator operator++(E) { Iterator tmp = *this; ++(*this); return tmp;
     } friend bool operator== (const Iterator& a, const Iterator& b) { return
     a.e_ptr == b.e_ptr; }; friend bool operator!= (const Iterator& a, const
     Iterator& b) { return a.e_ptr != b.e_ptr; };

      private:
          pointer e_ptr;
      };

      Iterator begin() { return Iterator(&m_data[0]); }
      Iterator end()   { return Iterator(&m_data[200]); }*/

  //////////////////////////////////////////////////////////////////////////////
  /////////////////////////    PRIVATE ///////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
private:
  const Bytecode<S, E> bcode;
  Fluent<S, E>(const Bytecode<S, E> &bcode) : bcode(bcode) {}
  template <typename E2>
  const Fluent<S, E2> addInst(const char *op, const List<void *> &args,
                              const Function<E, E2> function) const {
    return Fluent<S, E2>(bcode.addInst(op, args, function));
  }
};
//////////////////////////////////////////////////////////////////////////////
//////////////////////    STATIC HELPERS   ///////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

template <typename S> const static Fluent<S, S> __(const S &start) {
  return Fluent<S, S>(start);
};

template <typename S> const static Fluent<S, S> __() { return Fluent<S, S>(); };

} // namespace fhatos::kernel

#endif