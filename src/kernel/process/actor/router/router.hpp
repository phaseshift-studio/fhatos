#ifndef fhatos_kernel_router_hpp
#define fhatos_kernel_router_hpp

#include <fhatos.hpp>
#include <kernel/process/actor/message_box.hpp>
#include <kernel/structure/structure.hpp>

#define RETAIN_MESSAGE true

namespace fhatos::kernel {

///////////////////////////////////////////////////
/////////////// SUBSCRIPTION STRUCT ///////////////
///////////////////////////////////////////////////

template <typename MESSAGE>
using OnRecvFunction = std::function<void(const MESSAGE &)>;

enum QoS { _0 = 0, _1 = 1, _2 = 2, _3 = 3 };

template <typename MESSAGE> struct Subscription {
  MessageBox<Pair<const Subscription<MESSAGE>, const MESSAGE>> *actor;
  const ID source;
  const Pattern pattern;
  const QoS qos = _1;
  const OnRecvFunction<MESSAGE> onRecv;
  [[nodiscard]] bool match(const ID &target) const {
    return this->pattern.matches(target);
  }
  void execute(const MESSAGE message) const { onRecv(message); }
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

template <class MESSAGE> class Router : public IDed {

public:
  explicit Router(const ID &id) : IDed(id) {};
  virtual const RESPONSE_CODE publish(const MESSAGE &message) FP_OK_RESULT;
  virtual const RESPONSE_CODE
  subscribe(const Subscription<MESSAGE> &subscription) FP_OK_RESULT;
  virtual const RESPONSE_CODE unsubscribe(const ID &source,
                                          const Pattern &pattern) FP_OK_RESULT;
  virtual const RESPONSE_CODE unsubscribeSource(const ID &source) FP_OK_RESULT;
  virtual RESPONSE_CODE clear() FP_OK_RESULT;
  virtual RESPONSE_CODE registerRoute(const ID &source, const List<ID> incoming,
                                      const List<ID> outgoing) {
    return this->publish(MESSAGE{
        .source = source, .target = source, source.toString(), RETAIN_MESSAGE});
  }

  // virtual ID *adjacent(const ID &source) { return nullptr; }
  // virtual RESPONSE_CODE call(const ID &source, const ID &target)
  // FP_OK_RESULT;
};
} // namespace fhatos::kernel

#undef FP_OKAY_RESULT

#endif