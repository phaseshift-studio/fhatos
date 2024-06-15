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

#ifndef fhatos_publisher_hpp
#define fhatos_publisher_hpp

#include <process/actor/mailbox.hpp>
#include <process/router/message.hpp>
#include <process/router/router.hpp>

namespace fhatos {
  template<typename ROUTER>
  class Publisher {
  protected:
    ~Publisher() = default;

  public:
    const ID __id;
    Mailbox<ptr<Mail>> *mailbox;

    explicit Publisher(const IDed *ided, Mailbox<ptr<Mail>> *mailbox = nullptr) : __id(ided->id()), mailbox(mailbox) {}
    explicit Publisher(const ID &id, Mailbox<ptr<Mail>> *mailbox = nullptr) : __id(id), mailbox(mailbox) {}

    /// SUBSCRIBE
    virtual RESPONSE_CODE subscribe(const Pattern &relativePattern, const Consumer<const Message> onRecv,
                                    const QoS qos = QoS::_1) {
      return ROUTER::singleton()->subscribe(Subscription{.mailbox = this->mailbox,
                                                         .source = this->__id,
                                                         .pattern = makeTopic(relativePattern),
                                                         .qos = qos,
                                                         .onRecv = onRecv});
    }

    Publisher<ROUTER> *onQuery(const Pattern &queryPattern, const Runnable &runnable) {
      this->subscribe(queryPattern, [this, queryPattern, runnable](const Message &message) {
        if (!message.source.equals(this->__id) && message.isQuery(queryPattern.query().c_str()))
          runnable();
      });
      return this;
    }

    Publisher<ROUTER> *onQuery(const Pattern &queryPattern, const BiConsumer<SourceID, TargetID> &consumer) {
      this->subscribe(queryPattern, [this, queryPattern, consumer](const Message &message) {
        if (!message.source.equals(this->__id) && message.isQuery(queryPattern.query().c_str()))
          consumer(message.source, message.target);
      });
      return this;
    }

    Publisher<ROUTER> *onQuery(const Pattern &queryPattern, const Map<string, BiConsumer<SourceID, TargetID>> &mapping,
                               const BiConsumer<SourceID, TargetID> &thenDo = nullptr) {
      this->subscribe(queryPattern, [this, mapping, thenDo](const Message &message) {
        if (message.target.hasQuery() && !message.source.equals(this->__id)) {
          if (const string query = message.target.query(); mapping.count(query)) {
            mapping.at(query)(message.source, message.target);
          } else if (mapping.count("default")) {
            mapping.at("default")(message.source, message.target);
          }
          if (thenDo)
            thenDo(message.source, message.target);
        }
      });
      return this;
    }

    Publisher<ROUTER> *onQuery(const Pattern &queryPattern, const Map<string, Obj> &mapping) {
      Map<string, BiConsumer<SourceID, TargetID>> map;
      for (const auto pair: mapping) {
        map.insert({{pair.first, [this, pair](const SourceID source, const TargetID target) {
                       this->publish(target, BinaryObj<>::fromObj(&pair.second), RETAIN_MESSAGE);
                     }}});
      }
      return this->onQuery(queryPattern, map);
    }

    /// UNSUBSCRIBE
    virtual RESPONSE_CODE unsubscribe(const Pattern &relativePattern) {
      return ROUTER::singleton()->unsubscribe(this->__id, makeTopic(relativePattern));
    }

    virtual RESPONSE_CODE unsubscribeSource() { return ROUTER::singleton()->unsubscribeSource(this->__id); }

    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////

    /// PUBLISH
    const RESPONSE_CODE publish(const ID &relativeTarget, const BinaryObj<> *payload,
                                const bool retain = TRANSIENT_MESSAGE) const {
      return ROUTER::singleton()->publish(
          Message{.source = this->__id, .target = makeTopic(relativeTarget), .payload = payload->toObj(), .retain = retain});
    }

    /////////////////////////////////////////////////////////////////////////////////////////

    template<class T, class = typename std::enable_if_t<std::is_same_v<bool, T>>>
    const RESPONSE_CODE publish(const ID &relativeTarget, const bool payload,
                                const bool retain = TRANSIENT_MESSAGE) const {
      return this->publish(relativeTarget, new BinaryObj<>(payload), retain);
    }

    const RESPONSE_CODE publish(const ID &relativeTarget, const int payload,
                                const bool retain = TRANSIENT_MESSAGE) const {
      return this->publish(relativeTarget, new BinaryObj<>(payload), retain);
    }

    const RESPONSE_CODE publish(const ID &relativeTarget, const long payload,
                                const bool retain = TRANSIENT_MESSAGE) const {
      return this->publish(relativeTarget, new BinaryObj<>((int) payload), retain);
    }

    const RESPONSE_CODE publish(const ID &relativeTarget, const float payload,
                                const bool retain = TRANSIENT_MESSAGE) const {
      return this->publish(relativeTarget, new BinaryObj<>(payload), retain);
    }

    const RESPONSE_CODE publish(const ID &relativeTarget, const double payload,
                                const bool retain = TRANSIENT_MESSAGE) const {
      return this->publish(relativeTarget, new BinaryObj<>((float) payload), retain);
    }

    const RESPONSE_CODE publish(const ID &relativeTarget, const char *payload,
                                const bool retain = TRANSIENT_MESSAGE) const {
      return this->publish(relativeTarget, new BinaryObj<>(string(payload)), retain);
    }

    const RESPONSE_CODE publish(const ID &relativeTarget, const string &payload,
                                const bool retain = TRANSIENT_MESSAGE) const {
      return this->publish(relativeTarget, new BinaryObj<>(payload), retain);
    }

    const RESPONSE_CODE publish(const ID &relativeTarget, const OBJ_OR_BYTECODE &payload,
                                const bool retain = TRANSIENT_MESSAGE) const {
      return this->publish(relativeTarget, BinaryObj<>::fromObj(payload), retain);
    }

    //////////

    const RESPONSE_CODE write(const Obj *obj, const TargetID &relativeTarget) const {
      return this->publish(relativeTarget, BinaryObj<>::fromObj(obj), RETAIN_MESSAGE);
    }

    template<typename OBJ = Obj>
    const OBJ *read(const ID &relativeTarget) const {
      auto *thing = new std::atomic<OBJ *>(nullptr);
      auto *done = new std::atomic<bool>(false);
      this->subscribe(relativeTarget, [thing, done](const Message &message) {
        thing->store((OBJ *) message.payload);
        done->store(true);
      });
      const time_t startTimestamp = time(nullptr);
      while (!done->load()) {
        if (time(nullptr) - startTimestamp > 5) {
          LOG(ERROR, "Read timeout on target !y%s!!\n", relativeTarget.toString().c_str());
          break;
        }
      }
      const OBJ *ret = thing->load();
      delete thing;
      delete done;
      unsubscribe(relativeTarget);
      return ret;
    }

  private:
    const Pattern makeTopic(const Pattern &relativeTopic) const {
      return relativeTopic.empty() ? Pattern(this->__id)
                                   : (relativeTopic.toString()[0] == '/'
                                          ? Pattern(this->__id.toString() + "/" + relativeTopic.toString().substr(1))
                                          : relativeTopic)
                                         .resolve(this->__id);
    }
  };
} // namespace fhatos

#endif
