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
#pragma once
#ifndef fhatos_router_hpp
#define fhatos_router_hpp

#include <fhatos.hpp>
//
#include <process/actor/mailbox.hpp>
#include <process/router/message.hpp>
#include <structure/furi.hpp>
#include <util/enums.hpp>
#ifndef NATIVE
// #include <structure/io/net/f_wifi.hpp>
#else
#endif
#include FOS_PROCESS(coroutine.hpp)
#include FOS_PROCESS(thread.hpp)

#define RETAIN_MESSAGE true
#define TRANSIENT_MESSAGE false

#define LOG_SUBSCRIBE(rc, subscription)                                                                                \
  LOG(((rc) == OK ? DEBUG : ERROR), "!m[!!%s!m][!b%s!m]=!gsubscribe!m[qos:%i]=>[!b%s!m]!! | !m[onRecv:!!%s!m]!!\n",    \
      (string((rc) == OK ? "!g" : "!r") + RESPONSE_CODE_STR(rc) + "!!").c_str(),                                       \
      (subscription)->source.toString().c_str(), (uint8_t) (subscription)->qos,                                        \
      (subscription)->pattern.toString().c_str(),                                                                      \
      (subscription)->onRecvBCode ? (subscription)->onRecvBCode->toString().c_str() : "!bc/c++_impl!!")
#define LOG_UNSUBSCRIBE(rc, source, pattern)                                                                           \
  LOG(((rc) == OK ? DEBUG : ERROR), "!m[!!%s!m][!b%s!m]=!gunsubscribe!m=>[!b%s!m]!!\n",                                \
      (string((rc) == OK ? "!g" : "!r") + RESPONSE_CODE_STR(rc) + "!!").c_str(), ((source).toString().c_str()),        \
      nullptr == (pattern) ? "ALL" : (pattern)->toString().c_str())
#define LOG_PUBLISH(rc, message)                                                                                       \
  LOG(((rc) == OK ? DEBUG : WARN), "!m[!!%s!m][!b%s!m]=!gpublish!m[retain:%s]!b=!!%s!b=>!m[!b%s!m]!!\n",               \
      (string((rc) == OK ? "!g" : "!r") + RESPONSE_CODE_STR(rc) + "!!").c_str(),                                       \
      ((message).source.toString().c_str()), (FOS_BOOL_STR((message).retain)),                                         \
      ((message).payload->toString().c_str()), ((message).target.toString().c_str()))
#define LOG_RECEIVE(rc, subscription, message)                                                                         \
  LOG(((rc) == OK ? DEBUG : ERROR),                                                                                    \
      (((subscription).pattern.equals((message).target))                                                               \
           ? "!m[!!%s!m][!b%s!m]<=!greceive!m[pattern|target:!b%s!m]=!!%s!m=[!b%s!m]!!\n"                              \
           : "!m[!!%s!m][!b%s!m]<=!greceive!m[pattern:%s][target:%s]=!!%s!m=[!b%s!m]!!\n"),                            \
      (string((rc) == OK ? "!g" : "!r") + RESPONSE_CODE_STR(rc) + "!!").c_str(),                                       \
      ((subscription).source.toString().c_str()), ((subscription).pattern.toString().c_str()),                         \
      ((subscription).pattern.equals((message).target)) ? ((message).payload->toString().c_str())                      \
                                                        : ((message).target.toString().c_str()),                       \
      ((subscription).pattern.equals((message).target)) ? ((message).source.toString().c_str())                        \
                                                        : ((message).payload->toString)().c_str(),                     \
      ((message).source.toString().c_str()))

namespace fhatos {
  ///////////////////////////////////////////////////
  /////////////// SUBSCRIPTION STRUCT ///////////////
  ///////////////////////////////////////////////////
  enum class QoS { _0 = 0, _1 = 1, _2 = 2, _3 = 3 };

  struct Subscription;
  using Subscription_p = ptr<Subscription>;
  using Message_p = ptr<Message>;
  struct Subscription {
    using Mail = Pair<const Subscription_p, const Message_p>;
    using Mail_p = ptr<Mail>;
    Mailbox<Mail_p> *mailbox;
    ID source;
    Pattern pattern;
    QoS qos = QoS::_1;
    Consumer<const Message_p> onRecv = [](const Message_p &) {};
    BCode_p onRecvBCode = nullptr;

    bool match(const ID &target) const { return this->pattern.matches(target); }

    void execute(const Message_p &message) const { onRecv(message); }
  };


  using Mail = Pair<const Subscription_p, const Message_p>;
  using Mail_p = ptr<Mail>;
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

  enum class ROUTER_LEVEL { BCODE_ROUTER = 0, LOCAL_ROUTER = 1, GLOBAL_ROUTER = 2, META_ROUTER = 3 };
  static const Enums<ROUTER_LEVEL> ROUTER_LEVELS = Enums<ROUTER_LEVEL>({{ROUTER_LEVEL::BCODE_ROUTER, "bcode_router"},
                                                                        {ROUTER_LEVEL::LOCAL_ROUTER, "local_router"},
                                                                        {ROUTER_LEVEL::GLOBAL_ROUTER, "global_router"},
                                                                        {ROUTER_LEVEL::META_ROUTER, "meta_router"}});
  // template<typename PROCESS = Coroutine>
  class Router : public Coroutine {
  protected:
    explicit Router(const ID &id = ID("router"), const ROUTER_LEVEL level = ROUTER_LEVEL::LOCAL_ROUTER) :
        Coroutine(id), _level(level) {
      LOG(DEBUG, "Starting router: %s\n", id.toString().c_str());
    }

  public:
    ~Router() override = default;
    ROUTER_LEVEL _level;

    static ID mintID(const char *authority, const char *path = "") { return ID(ID(authority).path(path)); }

    virtual RESPONSE_CODE publish(const Message &) = 0;
    virtual RESPONSE_CODE subscribe(const Subscription &) = 0;
    virtual RESPONSE_CODE unsubscribe(const ID &, const Pattern &) = 0;
    virtual RESPONSE_CODE unsubscribeSource(const ID &) = 0;
    virtual RESPONSE_CODE clear() = 0;
    virtual uint retainSize() const { return -1; }
    virtual const string toString() const { return "Router"; }

    ///////////////////////////////////////////

    static Router *current() { return GLOBAL_OPTIONS->router<Router>(); }

    template<typename OBJ = Obj>
    static ptr<OBJ> read(const ID &target, const ID &source = FOS_DEFAULT_SOURCE_ID) {
      auto *router = GLOBAL_OPTIONS->router<Router>();
      auto *thing = new std::atomic<const Obj *>(nullptr);
      router->subscribe(Subscription{.source = source, .pattern = target, .onRecv = [thing](const Message_p &message) {
                                       // TODO: try to not copy obj while still not accessing heap after delete
                                       const Obj *obj = new Obj(message->payload->_value, message->payload->id());
                                       thing->store(obj);
                                     }});
      const time_t startTimestamp = time(nullptr);
      while (!thing->load()) {
        if ((time(nullptr) - startTimestamp) > static_cast<uint8_t>(router->_level) + 1) {
          break;
        }
      }
      router->unsubscribe(source, target);
      if (nullptr == thing->load()) {
        delete thing;
        return Obj::to_noobj();
      } else {
        const ptr<OBJ> ret = ptr<OBJ>((OBJ *) thing->load());
        delete thing;
        return ret;
      }
    }

    static Rec_p readPattern(const Pattern &target, const ID &source = FOS_DEFAULT_SOURCE_ID) {
      auto *router = GLOBAL_OPTIONS->router<Router>();
      auto *map = new Obj::RecMap<>();
      router->subscribe(Subscription{.source = source, .pattern = target, .onRecv = [map](const Message_p &message) {
                                       map->insert({Obj::to_uri(message->target), message->payload});
                                     }});
      const time_t startTimestamp = time(nullptr);
      while ((time(nullptr) - startTimestamp) < static_cast<uint8_t>(router->_level)) {
      }
      router->unsubscribe(source, target);
      return Obj::to_rec(Obj::RecMap_p<>(map));
    }

    static RESPONSE_CODE write(const ID &target, const Obj_p &obj, const ID &source = FOS_DEFAULT_SOURCE_ID) {
      return GLOBAL_OPTIONS->router<Router>()->publish(
          Message{.source = source, .target = target, .payload = obj, .retain = RETAIN_MESSAGE});
    }

    static RESPONSE_CODE destroy(const ID &target, const ID &source = FOS_DEFAULT_SOURCE_ID) {
      return GLOBAL_OPTIONS->router<Router>()->publish(
          Message{.source = source, .target = target, .payload = Obj::to_noobj(), .retain = RETAIN_MESSAGE});
    }
  };
} // namespace fhatos

#undef FP_OKAY_RESULT

#endif
