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


#ifndef fhatos_structure_subscription_hpp
#define fhatos_structure_subscription_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>

namespace fhatos {
#define RETAIN_MESSAGE true
#define TRANSIENT_MESSAGE false

#define LOG_SUBSCRIBE(rc, subscription)                                                                                \
  LOG(((rc) == OK ? DEBUG : ERROR), "!m[!!%s!m][!b%s!m]=!gsubscribe!m[qos:%i]=>[!b%s!m]!! | !m[onRecv:!!%s!m]!!\n",    \
      (string((rc) == OK ? "!g" : "!r") + ResponseCodes.toChars(rc) + "!!").c_str(),                                   \
      (subscription)->source.toString().c_str(), (uint8_t) (subscription)->qos,                                        \
      (subscription)->pattern.toString().c_str(),                                                                      \
      (subscription)->onRecv->toString().c_str())
#define LOG_UNSUBSCRIBE(rc, source, pattern)                                                                           \
  LOG(((rc) == OK ? DEBUG : ERROR), "!m[!!%s!m][!b%s!m]=!gunsubscribe!m=>[!b%s!m]!!\n",                                \
      (string((rc) == OK ? "!g" : "!r") + ResponseCodes.toChars(rc) + "!!").c_str(), ((source).toString().c_str()),    \
      nullptr == (pattern) ? "ALL" : (pattern)->toString().c_str())
#define LOG_PUBLISH(rc, message)                                                                                       \
  LOG(((rc) == OK ? DEBUG : WARN), "!m[!!%s!m][!b%s!m]=!gpublish!m[retain:%s]!b=!!%s!b=>!m[!b%s!m]!!\n",               \
      (string((rc) == OK ? "!g" : "!r") + ResponseCodes.toChars(rc) + "!!").c_str(),                                   \
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

  static Enums<RESPONSE_CODE> ResponseCodes = Enums<RESPONSE_CODE>({
    {OK, "OK"},
    {NO_TARGETS, "no targets"},
    {REPEAT_SUBSCRIPTION, "repeat subscription"},
    {NO_SUBSCRIPTION, "no subscription"},
    {NO_MESSAGE, "no message"},
    {ROUTER_ERROR, "internal router error"},
    {MUTEX_TIMEOUT, "router timeout"}
  });

  //////////////////////////////////////////////
  /////////////// MESSAGE STRUCT ///////////////
  //////////////////////////////////////////////
  struct Message {
    using Message_p = ptr<Message>;

  public:
    const ID source;
    const ID target;
    const Obj_p payload;
    const bool retain;

    [[nodiscard]] string toString() const {
      char temp[250];
      snprintf(temp, 250, "!g[!b%s!g]!!=!y%s!![retain:%s]=>!g[!b%s!g]!!", this->source.toString().c_str(),
               this->payload->toString().c_str(), FOS_BOOL_STR(this->retain), this->target.toString().c_str());
      return {temp};
    }

    [[nodiscard]] Rec_p to_rec() const {
      return rec({
        {uri("source"), uri(source)},
        {uri("target"), uri(target)},
        {uri("payload"), payload},
        {uri("retain"), dool(retain)}
      });
    }

    /////////////// HELPER METHODS TO HANDLE ROUTERS THAT DON'T PROPAGATE SOURCE (e.g. MQTT) ///////////////
    static BObj_p wrap_source(const ID_p &source, const Obj_p &obj) {
      string wrap = source->toString();
      wrap += '%';
      wrap += obj->is_uri() ? ("<" + obj->toString(true, false) + ">") : obj->toString(true, false); // TODO: figure out general URI serialization using toString() as <3> doesn't work
      LOG(TRACE, "bobj source wrap: %s (length:%i)\n", wrap.c_str(), wrap.length());
      return ptr<BObj>(new BObj({wrap.length(), reinterpret_cast<fbyte *>(strdup(wrap.c_str()))}), bobj_deleter);
    }

    static Pair<ID_p, Obj_p> unwrap_source(const BObj_p &bobj) {
      try {
        const auto unwrap = string(reinterpret_cast<char *>(bobj->second), bobj->first);
        const size_t index = unwrap.find_first_of('%');
        LOG(TRACE, "bobj source unwrap: %s and %s (length:%i and %i)\n", unwrap.substr(0, index).c_str(),
            unwrap.substr(index + 1).c_str(), unwrap.substr(0, index).length(), unwrap.substr(index + 1).length());
        if (index == string::npos)
          throw fError("bobj is not wrapped with source: %s\n", bobj->second);
        return Pair<ID_p, Obj_p>(
          {id_p(unwrap.substr(0, index).c_str()), Options::singleton()->parser<Obj>(unwrap.substr(index + 1))});
      } catch (const std::exception &e) {
        LOG_EXCEPTION(e);
        throw;
      }
    }
  };

  ///////////////////////////////////////////////////
  /////////////// SUBSCRIPTION STRUCT ///////////////
  ///////////////////////////////////////////////////
  enum class QoS {
    _0 = 0, _1 = 1, _2 = 2, _3 = 3
  };

  struct Subscription;
  using Subscription_p = ptr<Subscription>;
  using Message_p = ptr<Message>;
  using Mail = Pair<const Subscription_p, const Message_p>;
  using Mail_p = ptr<Mail>;

  struct Mailbox {
  public:
    virtual ~Mailbox() = default;

    virtual bool recv_mail(const Mail_p &mail) = 0;
  };

  struct Subscription {
    fURI source;
    Pattern pattern;
    QoS qos = QoS::_1;
    BCode_p onRecv = bcode();

    [[nodiscard]] Rec_p to_rec() const {
      return rec({
        {uri("source"), uri(source)},
        {uri("pattern"), uri(pattern)},
        {uri("qos"), jnt(static_cast<int>(qos))},
        {uri("onRecv"), onRecv}
      });
    }

    [[nodiscard]] string toString() const {
      char temp[250];
      snprintf(temp, 250, "[!b%s!m]=!gsubscribe!m[qos:%i]=>[!b%s!m]!! | !m[onRecv:!!%s!m]!!", source.toString().c_str(),
               static_cast<uint8_t>(qos), pattern.toString().c_str(), onRecv->toString().c_str());
      return {temp};
    }
  };
} // namespace fhatos

#endif
