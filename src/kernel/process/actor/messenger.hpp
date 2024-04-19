#ifndef fhatos_kernel__messenger_hpp
#define fhatos_kernel__messenger_hpp

#include <fhatos.hpp>
#include <kernel/structure/structure.hpp>

namespace fhatos::kernel {

template <typename T> class Messenger : public IDed {

public:
  Messenger(const ID& id) : IDed(id) {};
  virtual bool push(const T &message);
  virtual Option<T> pop();
  virtual uint16_t mailboxSize() const;
};
} // namespace fatpig

#endif