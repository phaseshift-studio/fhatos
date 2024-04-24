#ifndef fhatos_kernel__message_box_hpp
#define fhatos_kernel__message_box_hpp

#include <fhatos.hpp>
#include <kernel/structure/structure.hpp>

namespace fhatos::kernel {

template <typename T> struct MessageBox {
  virtual bool push( T* message);
  virtual Option<T*> pop();
  virtual uint16_t size() const;
};
} // namespace fhatos::kernel

#endif