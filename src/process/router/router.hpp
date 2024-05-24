#ifndef fhatos_router_hpp
#define fhatos_router_hpp

#include <fhatos.hpp>
//
#include <structure/furi.hpp>
#include <process/actor/mailbox.hpp>
#include <process/router/message.hpp>
#include FOS_PROCESS(thread.hpp)

#define RETAIN_MESSAGE true
#define TRANSIENT_MESSAGE false

namespace fhatos {
  ///////////////////////////////////////////////////
  /////////////// SUBSCRIPTION STRUCT ///////////////
  ///////////////////////////////////////////////////

  enum QoS { _0 = 0, _1 = 1, _2 = 2, _3 = 3 };

  struct Subscription {
    using Mail = Pair<const Subscription, const Message>;
    Mailbox<Mail> *mailbox;
    ID source;
    Pattern pattern;
    QoS qos = _1;
    Consumer<const Message> onRecv;

    const bool match(const ID &target) const {
      return this->pattern.matches(target);
    }

    void execute(const Message &message) const { onRecv(message); }
  };

  using Mail = Pair<const Subscription, const Message>;
  //////////////////////////////////////////////
  /////////////// ERROR MESSAGES ///////////////
  //////////////////////////////////////////////

  enum RESPONSE_CODE {
    OK = 0,
    NO_TARGETS,
    REPEAT_SUBSCRIPTION,
    NO_SUBSCRIPTION,
    NO_MESSAGE,
    ROUTER_ERROR,
    MUTEX_TIMEOUT,
    MUTEX_LOCKOUT
  };

  static const char *RESPONSE_CODE_STR(const RESPONSE_CODE &rc) {
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
        return (new string(string("Unknown error code: ") + std::to_string(rc)))->c_str();
    }
  };

  ////////////////////////////////////////////
  /////////////// ROUTER CLASS ///////////////
  ////////////////////////////////////////////

#define FP_OK_RESULT                                                           \
  { return RESPONSE_CODE::OK; }

  template<typename PROCESS = Thread>
  class Router : public PROCESS {
  public:
    explicit Router(const ID &id) : PROCESS(id) {
    };
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
} // namespace fhatos

#undef FP_OKAY_RESULT

#endif
