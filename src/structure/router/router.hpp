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
#include <structure/furi.hpp>
#include <structure/router/pubsub_artifacts.hpp>
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
      (string((rc) == OK ? "!g" : "!r") + ResponseCodes.toChars(rc) + "!!").c_str(),                                       \
      (subscription)->source.toString().c_str(), (uint8_t) (subscription)->qos,                                        \
      (subscription)->pattern.toString().c_str(),                                                                      \
      (subscription)->onRecvBCode ? (subscription)->onRecvBCode->toString().c_str() : "!bc/c++_impl!!")
#define LOG_UNSUBSCRIBE(rc, source, pattern)                                                                           \
  LOG(((rc) == OK ? DEBUG : ERROR), "!m[!!%s!m][!b%s!m]=!gunsubscribe!m=>[!b%s!m]!!\n",                                \
      (string((rc) == OK ? "!g" : "!r") + ResponseCodes.toChars(rc) + "!!").c_str(), ((source).toString().c_str()),        \
      nullptr == (pattern) ? "ALL" : (pattern)->toString().c_str())
#define LOG_PUBLISH(rc, message)                                                                                       \
  LOG(((rc) == OK ? DEBUG : WARN), "!m[!!%s!m][!b%s!m]=!gpublish!m[retain:%s]!b=!!%s!b=>!m[!b%s!m]!!\n",               \
      (string((rc) == OK ? "!g" : "!r") + ResponseCodes.toChars(rc) + "!!").c_str(),                                       \
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

  static Enums<RESPONSE_CODE> ResponseCodes = Enums<RESPONSE_CODE>({{OK, "OK"},
                                                                    {NO_TARGETS, "no targets"},
                                                                    {REPEAT_SUBSCRIPTION, "repeat subscription"},
                                                                    {NO_SUBSCRIPTION, "no subscription"},
                                                                    {NO_MESSAGE, "no message"},
                                                                    {ROUTER_ERROR, "internal router error"},
                                                                    {MUTEX_TIMEOUT, "router timeout"}});

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
    virtual ~Router() override = default;
    ROUTER_LEVEL _level;

    static ID mintID(const char *authority, const char *path = "") { return ID(ID(authority).path(path)); }

    virtual RESPONSE_CODE publish(const Message &) = 0;
    virtual RESPONSE_CODE subscribe(const Subscription &) = 0;
    virtual RESPONSE_CODE unsubscribe(const ID &, const Pattern &) = 0;
    virtual RESPONSE_CODE unsubscribeSource(const ID &) = 0;
    virtual RESPONSE_CODE clear(bool subscriptions = true, bool retains = true) = 0;
    virtual uint retainSize() const { return -1; }
    virtual const string toString() const { return "Router"; }

    ///////////////////////////////////////////

    template<typename OBJ = Obj>
    static ptr<OBJ> read(const ID &target, const ID &source = FOS_DEFAULT_SOURCE_ID) {
      auto *router = Options::singleton()->router<Router>();
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
      router->unsubscribe(source, Pattern(target));
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
      auto *router = Options::singleton()->router<Router>();
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
      return Options::singleton()->router<Router>()->publish(
          Message{.source = source, .target = target, .payload = obj, .retain = RETAIN_MESSAGE});
    }

    static RESPONSE_CODE destroy(const ID &target, const ID &source = FOS_DEFAULT_SOURCE_ID) {
      return Options::singleton()->router<Router>()->publish(
          Message{.source = source, .target = target, .payload = Obj::to_noobj(), .retain = RETAIN_MESSAGE});
    }
    /////////////// HELPER METHODS TO HANDLE ROUTERS THAT DON'T PROPAGATE SOURCE (e.g. MQTT) ///////////////
    static BObj_p wrapSource(const SourceID &source, const Obj_p &obj) {
      string wrap = source.toString();
      wrap += '%';
      wrap += obj->toString(true, false);
      LOG(TRACE, "bobj source wrap: %s (length:%i)\n", wrap.c_str(), wrap.length());
      return ptr<BObj>(new BObj({wrap.length(), (fbyte *) strdup(wrap.c_str())}), bobj_deleter);
    }
    static Pair<SourceID, Obj_p> unwrapSource(const BObj_p &bobj) {
      try {
        const auto unwrap = string((char *) bobj->second, bobj->first);
        const size_t index = unwrap.find_first_of('%');
        LOG(TRACE, "bobj source unwrap: %s and %s (length:%i and %i)\n", unwrap.substr(0, index).c_str(),
            unwrap.substr(index + 1).c_str(), unwrap.substr(0, index).length(), unwrap.substr(index + 1).length());
        if (index == string::npos)
          throw fError("bobj is not wrapped with source: %s\n", bobj->second);
        return Pair<ID, Obj_p>(
            {ID(unwrap.substr(0, index)), Options::singleton()->parser<Obj>(unwrap.substr(index + 1))});
      } catch (const std::exception &e) {
        LOG_EXCEPTION(e);
        throw;
      }
    }
  };
} // namespace fhatos

#undef FP_OKAY_RESULT

#endif
