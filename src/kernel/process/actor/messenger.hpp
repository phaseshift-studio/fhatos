#ifndef fhatos_kernel__messenger_hpp
#define fhatos_kernel__messenger_hpp

#include <fhatos.hpp>

namespace fhatos::kernel {

template <typename MESSAGE> class Messenger {

public:
  virtual bool push(const MESSAGE &message);
  virtual Option<MESSAGE> pop();
  virtual uint16_t mailboxSize() const;
};
} // namespace fatpig

#endif