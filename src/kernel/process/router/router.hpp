#ifndef fhatos_kernel__router_hpp
#define fhatos_kernel__router_hpp

#include <fhatos.hpp>
//
#include <kernel/furi.hpp>
#include <kernel/process/actor/message_box.hpp>
#include FOS_PROCESS(thread.hpp)

#define RETAIN_MESSAGE true
#define TRANSIENT_MESSAGE false

namespace fhatos::kernel {

//////////////////////////////////////////////
/////////////// MESSAGE STRUCT ///////////////
//////////////////////////////////////////////

enum MType { OBJ, BOOL, INT, REAL, STR, LST, REC };
struct Payload {
  const MType type;
  const byte *data;
  const uint length;
};

struct Message {
public:
  const ID source;
  const ID target;
  const Payload payload;
  const bool retain;

  /*Message(ID source, ID target, PAYLOAD payload, const bool retain = false)
      : source(std::move(source)), target(std::move(target)),
        payload(std::move(payload)), retain(retain) {};*/

  const String toString() const {
    char temp[100];
    sprintf(temp, "[%s]=%s[retain:%s]=>[%s]", source.toString().c_str(),
            payloadString().c_str(), FP_BOOL_STR(retain),
            target.toString().c_str());
    return temp;
  };

  const String payloadString() const {
    switch (this->payload.type) {
    case BOOL:
      return FP_BOOL_STR(this->payload.data[0] == (byte)'1');
    case INT: {
      int integer = 0;
      integer = (integer << 8) + this->payload.data[3];
      integer = (integer << 8) + this->payload.data[2];
      integer = (integer << 8) + this->payload.data[1];
      integer = (integer << 8) + this->payload.data[0];
      return "" + integer;
    }
    case STR:
      return String(this->payload.data, this->payload.length);
    default:
      throw fError("Unknown type: %i", this->payload.type);
    }
  }
};

///////////////////////////////////////////////////
/////////////// SUBSCRIPTION STRUCT ///////////////
///////////////////////////////////////////////////

enum QoS { _0 = 0, _1 = 1, _2 = 2, _3 = 3 };
struct Subscription {
  MessageBox<Pair<const Subscription, const Message>> *actor;
  const ID source;
  const Pattern pattern;
  const QoS qos = _1;
  const Consumer<Message> onRecv;
  const bool match(const ID &target) const {
    return this->pattern.matches(target);
  }
  void execute(const Message &message) const { onRecv(message); }
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
  ROUTER_ERROR = 5,
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
  case ROUTER_ERROR:
    return "Router error";
  case MUTEX_TIMEOUT:
    return "Mutex timeout";
  default:
    return &"Unknown error code: "[rc];
  }
};

////////////////////////////////////////////
/////////////// ROUTER CLASS ///////////////
////////////////////////////////////////////

#define FP_OK_RESULT                                                           \
  { return RESPONSE_CODE::OK; }

template <typename PROCESS = Thread> class Router : public PROCESS {

public:
  explicit Router(const ID &id) : PROCESS(id) {};
  virtual const RESPONSE_CODE publish(const Message &message) FP_OK_RESULT;
  virtual const RESPONSE_CODE
  subscribe(const Subscription &subscription) FP_OK_RESULT;
  virtual const RESPONSE_CODE unsubscribe(const ID &source,
                                          const Pattern &pattern) FP_OK_RESULT;
  virtual const RESPONSE_CODE unsubscribeSource(const ID &source) FP_OK_RESULT;
  virtual RESPONSE_CODE clear() FP_OK_RESULT;

  // virtual ID *adjacent(const ID &source) { return nullptr; }
  // virtual RESPONSE_CODE call(const ID &source, const ID &target)
  // FP_OK_RESULT;
};
} // namespace fhatos::kernel

#undef FP_OKAY_RESULT

#endif