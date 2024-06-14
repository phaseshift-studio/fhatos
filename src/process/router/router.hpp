/*******************************************************************************
 FhatOS: A Distributed Operating System
 Copyright (c) 2024 PhaseShift Studio, LLC

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef fhatos_router_hpp
#define fhatos_router_hpp

#include <fhatos.hpp>
//
#include <process/actor/mailbox.hpp>
#include <process/router/message.hpp>
#include <structure/furi.hpp>
#ifndef NATIVE
#include <structure/io/net/f_wifi.hpp>
#endif
#include FOS_PROCESS(thread.hpp)

#define RETAIN_MESSAGE true
#define TRANSIENT_MESSAGE false

namespace fhatos {
  ///////////////////////////////////////////////////
  /////////////// SUBSCRIPTION STRUCT ///////////////
  ///////////////////////////////////////////////////

  enum class QoS { _0 = 0, _1 = 1, _2 = 2, _3 = 3 };

  struct Subscription {
    using Mail = Pair<const ptr<Subscription>, const ptr<Message>>;
    Mailbox<ptr<Mail>> *mailbox;
    ID source;
    Pattern pattern;
    QoS qos = QoS::_1;
    Consumer<const Message &> onRecv;

    const bool match(const ID &target) const { return this->pattern.matches(target); }

    void execute(const Message &message) const { onRecv(message); }
  };

  using Mail = Pair<const ptr<Subscription>, const ptr<Message>>;
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

#define FP_OK_RESULT                                                                                                   \
  { return RESPONSE_CODE::OK; }

  class Router {
  public:
    static ID mintID(const char *user, const char *path = "") {
#ifdef NATIVE
      return ID(path).user(user);
#else
      return fWIFI::idFromIP(user, path);
#endif
    }

    virtual const RESPONSE_CODE publish(const Message &message) FP_OK_RESULT;

    virtual const RESPONSE_CODE subscribe(const Subscription &subscription) FP_OK_RESULT;

    virtual const RESPONSE_CODE unsubscribe(const ID &source, const Pattern &pattern) FP_OK_RESULT;
    virtual const RESPONSE_CODE unsubscribeSource(const ID &source) FP_OK_RESULT;
    virtual const RESPONSE_CODE clear() FP_OK_RESULT;

    template<typename OBJ = Obj>
    const OBJ *read(const ID &source, const ID &target) {
      auto *thing = new std::atomic<OBJ *>(nullptr);
      auto *done = new std::atomic<bool>(false);
      this->subscribe(
          Subscription{.source = source, .pattern = target, .onRecv = [thing, done](const Message &message) {
                         thing->store((OBJ *) message.payload->toObj());
                         done->store(true);
                       }});
      const time_t startTimestamp = time(nullptr);
      while (!done->load()) {
        if (time(nullptr) - startTimestamp > 5) {
          LOG(ERROR, "Read timeout on target !y%s!!\n", target.toString().c_str());
          break;
        }
      }
      const OBJ *ret = thing->load();
      delete thing;
      delete done;
      unsubscribe(source, target);
      return ret;
    }

    virtual RESPONSE_CODE write(const Obj *obj, const ID &source, const ID &target) {
      return this->publish(
          Message{.source = source, .target = target, .payload = BinaryObj<>::fromObj(obj), .retain = RETAIN_MESSAGE});
    }
  };
} // namespace fhatos

#undef FP_OKAY_RESULT

#endif
