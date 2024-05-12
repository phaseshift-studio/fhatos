#ifndef fhatos_mailbox_hpp
#define fhatos_mailbox_hpp

#include <fhatos.hpp>

namespace fhatos {
  template<typename T>
  struct Mailbox {
  public:
    virtual const bool push(const T message) { return true; }

    virtual const uint16_t size() const { return 0; }

  protected:
    virtual const Option<T> pop() { return {}; }
  };
} // namespace fhatos

#endif
