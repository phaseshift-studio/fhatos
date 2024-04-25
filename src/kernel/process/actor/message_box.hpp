#ifndef fhatos_kernel__message_box_hpp
#define fhatos_kernel__message_box_hpp

#include <fhatos.hpp>
#include <kernel/structure/structure.hpp>

namespace fhatos::kernel {

template <typename T> struct MessageBox {
public:
  virtual const bool push(const T& message);
  virtual const uint16_t size() const;

protected:
  virtual Option<T> pop();
};
} // namespace fhatos::kernel

#endif