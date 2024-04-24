#ifndef fhatos_kernel__message_box_hpp
#define fhatos_kernel__message_box_hpp

#include <fhatos.hpp>
#include <kernel/structure/structure.hpp>

namespace fhatos::kernel {

template <typename T> struct MessageBox {
  virtual const bool push(const T message);
  virtual Option<T> pop();
  virtual const uint16_t size() const;
};
} // namespace fhatos::kernel

#endif