#ifndef fhatos_kernel__broker_hpp
#define fhatos_kernel__broker_hpp

#include <fhatos.hpp>
#include <kernel/process/actor/message_box.hpp>
#include <kernel/structure/structure.hpp>
// #include FP_THREAD_LIBRARY

#define RETAIN_MESSAGE true

namespace fhatos::kernel {

//////////////////////////////////////////////
/////////////// MESSAGE STRUCT ///////////////
//////////////////////////////////////////////

template <typename M> struct Message {
  const ID source;
  const ID target;
  const M payload;
  const bool retain;
  const String payloadString() const {
    String p;
    serializeJson(payload, p);
    return p;
  }
  const Pair<byte *, int> toBytes() const {
    String temp = payloadString();
    byte *bytes = (byte *)temp.c_str();
    return std::make_pair(bytes, temp.length());
  }
  const String toString() const {
    char temp[100];
    sprintf(temp, "[%s]=%s=>[%s]", source.toString().c_str(), payloadString(),
            target.toString().c_str());
    return temp;
  }
};

struct StringMessage {
  const ID source;
  const ID target;
  const String payload;
  const bool retain;
  static const String fromBytes(const byte *bytes, const int length) {
    return String((char *)bytes);
  }
  const String payloadString() const { return this->payload; }
};

struct IntMessage : public Message<int> {
public:
  static int fromBytes(const byte *bytes, const int length) {
    return (int)(bytes[0] >> 0 & bytes[1] >> 8 & bytes[2] >> 16 &
                 bytes[3] >> 24);
  }
  const Pair<byte *, int> toBytes() const {
    unsigned char bytes[4];
    bytes[0] = (this->payload >> 24) & 0xFF;
    bytes[1] = (this->payload >> 16) & 0xFF;
    bytes[2] = (this->payload >> 8) & 0xFF;
    bytes[3] = (this->payload >> 0) & 0xFF;
    return std::make_pair(bytes, 4);
  }
};

////////////////////////////////////
/////////// JSON MESSAGE ///////////
////////////////////////////////////

class JSON : public JsonDocument {
public:
  JSON(const String &json) { deserializeJson(*this, json); }
  const String toString() const {
    String temp;
    serializeJson(*this, temp);
    return temp;
  }
};

struct JSONMessage {
  const ID source;
  const ID target;
  const JSON payload;
  const bool retain;
  static const JSON fromBytes(const byte *bytes, const int length) {
    return JSON(String((char *)bytes));
  }
  const Pair<byte *, int> toBytes() const {
    return Pair<byte *, int>{(byte *)payload.toString().c_str(),
                             payload.toString().length()};
  }
  const String toString() const {
    char temp[100];
    sprintf(temp, "[%s]=%s=>[%s]", source.toString().c_str(),
            this->payload.toString().c_str(), target.toString().c_str());
    return String(temp);
  }
};

///////////////////////////////////////////////////
/////////////// SUBSCRIPTION STRUCT ///////////////
///////////////////////////////////////////////////

template <typename MESSAGE>
using OnRecvFunction = std::function<void(const MESSAGE &)>;

enum QoS { _0 = 0, _1 = 1, _2 = 2, _3 = 3 };

template <typename MESSAGE> struct Subscription {
  MessageBox<Pair<Subscription<MESSAGE>, MESSAGE>> *actor;
  const ID source;
  const Pattern pattern;
  const QoS qos;
  const OnRecvFunction<MESSAGE> onRecv;
  bool match(const ID &target) { return this->pattern.matches(target); }
  void execute(const MESSAGE message) { onRecv(message); }
};

//////////////////////////////////////////////
/////////////// ERROR MESSAGES ///////////////
//////////////////////////////////////////////

enum RESPONSE_CODE {
  OK = 0,
  NO_TARGETS = 1,
  REPEAT_SUBSCRIPTION = 2,
  NO_SUBSCRIPTION = 3,
  NO_MESSAGE = 4,
  BROKER_ERROR = 5,
  MUTEX_TIMEOUT = 6
};

static String RESPONSE_CODE_STR(const RESPONSE_CODE rc) {
  switch (rc) {
  case OK:
    return "OK";
  case NO_TARGETS:
    return "No Targets";
  case REPEAT_SUBSCRIPTION:
    return "Subscription already exists";
  case NO_SUBSCRIPTION:
    return "Subscription doesn't exist";
  case NO_MESSAGE:
    return "No message";
  case BROKER_ERROR:
    return "Broker error";
  case MUTEX_TIMEOUT:
    return "Mutex timeout";
  default:
    return "Unknown error code: " + rc;
  }
};

////////////////////////////////////////////
/////////////// BROKER CLASS ///////////////
////////////////////////////////////////////

#define FP_OK_RESULT                                                           \
  { return RESPONSE_CODE::OK; }

template <class MESSAGE> class Broker : public IDed {

public:
  Broker(const ID &id) : IDed(id){};
  virtual RESPONSE_CODE publish(const MESSAGE &message) FP_OK_RESULT;
  virtual RESPONSE_CODE
  subscribe(const Subscription<MESSAGE> &subscription) FP_OK_RESULT;
  virtual RESPONSE_CODE unsubscribe(const ID &source,
                                    const Pattern &pattern) FP_OK_RESULT;
  virtual RESPONSE_CODE unsubscribeSource(const ID &source) FP_OK_RESULT;
  // virtual ID *adjacent(const ID &source) { return nullptr; }
  // virtual RESPONSE_CODE call(const ID &source, const ID &target)
  // FP_OK_RESULT;
};
} // namespace fhatos::kernel

#undef FP_OKAY_RESULT

#endif