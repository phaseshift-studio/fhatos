#ifndef fhatos_kernel__message_hpp
#define fhatos_kernel__message_hpp

#include <fhatos.hpp>
#include <kernel/structure/structure.hpp>

namespace fhatos::kernel {

template <typename PAYLOAD> class Message {

public:
  const ID source;
  const ID target;
  const PAYLOAD payload;
  const bool retain;
  Message(const ID &source, const ID &target, const PAYLOAD &payload,
          const bool retain = false)
      : source(source), target(target), payload(payload), retain(retain){};

  virtual const String toString() const {
    char temp[100];
    sprintf(temp, "[%s]=%s[retain:%s]=>[%s]", source.toString().c_str(),
            payloadString().c_str(), FP_BOOL_STR(retain),
            target.toString().c_str());
    return temp;
  };
  virtual const String payloadString() const { return ""; }

  virtual const Pair<byte *, uint> toBytes() const {
    String temp = payloadString();
    byte *bytes = (byte *)temp.c_str();
    return std::make_pair(bytes, temp.length());
  }
};

////////////////////////////////////////////////
//////////////// STRING MESSAGE ////////////////
////////////////////////////////////////////////

class StringMessage : public Message<String> {
public:
  StringMessage(const ID &source, const ID &target, const String &payload,
                const bool retain = false)
      : Message<String>(source, target, payload, retain){};
  static const String fromBytes(const byte *bytes, const uint length) {
    return String((char *)bytes, length);
  }
  virtual const Pair<byte *, uint> toBytes() const override {
    return std::make_pair((byte *)this->payload.c_str(),
                          this->payload.length());
  }
  virtual const String payloadString() const { return this->payload; }
};

} // namespace fhatos::kernel

#endif