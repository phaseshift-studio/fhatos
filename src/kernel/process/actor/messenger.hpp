#ifndef fhatos_kernel__messenger_hpp
#define fhatos_kernel__messenger_hpp

#include <fhatos.hpp>
///
#include <kernel/process/actor/broker/broker.hpp>

namespace fatpig {

template <typename MESSAGE> class Messenger {

public:
  virtual bool receive(const MESSAGE &message);
  virtual uint16_t mailboxSize() const;
};
} // namespace fatpig

#endif