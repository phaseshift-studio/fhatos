#ifndef fhatos_kernel__message_box_hpp
#define fhatos_kernel__message_box_hpp

#include <fhatos.hpp>
#include <kernel/structure/structure.hpp>

namespace fhatos::kernel {

template <typename T> class MessageBox : public IDed {

public:
  MessageBox(const ID& id) : IDed(id) {};
  virtual bool push(const T &message);
  virtual Option<T> pop();
  virtual uint16_t size() const;
};
} // namespace fatpig

#endif