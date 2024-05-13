#ifndef fhatos_mailbox_hpp
#define fhatos_mailbox_hpp

#include <fhatos.hpp>

namespace fhatos {
  template<typename T>
  struct Mailbox {
  public:
    virtual ~Mailbox() {
      this->clear();
    }

    void clear() {
      while (this->pop().has_value()) {
      }
    }

    virtual bool push(const T message) { return true; }

    virtual uint16_t size() const { return 0; }

  protected:
    virtual Option<T> pop() { return {}; }
  };
} // namespace fhatos

#endif
