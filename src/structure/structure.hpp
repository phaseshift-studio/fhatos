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
#ifndef fhatos_structure_hpp
#define fhatos_structure_hpp

#include <fhatos.hpp>
#include <language/obj.hpp>
#include <structure/pubsub.hpp>
#include <util/enums.hpp>
#include <util/mutex_deque.hpp>
#include <util/mutex_rw.hpp>

#define FOS_TRY_META \
const Option<Obj_p> meta = this->try_meta(furi,source); \
if (meta.has_value()) return meta.value();


namespace fhatos {
  class Router;

  enum class SType {
    EPHEMERAL, VARIABLES, DATABASE, HARDWARE, NETWORKED
  };

  static const Enums<SType> StructureTypes =
      Enums<SType>({
        {SType::EPHEMERAL, "ephemeral"},
        {SType::VARIABLES, "variables"},
        {SType::DATABASE, "database"},
        {SType::HARDWARE, "hardware"},
        {SType::NETWORKED, "networked"}
      });

  class Structure : public Patterned {
    friend class System;

  protected:
    ptr<MutexDeque<Mail_p>> outbox_ = std::make_shared<MutexDeque<Mail_p>>();
    ptr<List<Subscription_p>> subscriptions_ = std::make_shared<List<Subscription_p>>();
    MutexRW<> mutex_ = MutexRW<>();
    std::atomic_bool available_ = std::atomic_bool(false);
    bool remote_retains_ = false;

  public:
    const SType stype;

    explicit Structure(const Pattern &pattern, const SType stype): Patterned(p_p(pattern)), stype(stype) {
    }

    [[nodiscard]] bool available() const {
      return this->available_.load();
    }

    virtual void setup() {
      if (this->available_.load()) {
        LOG_STRUCTURE(WARN, this, "!ystructure!! already open");
        return;
      }
      this->available_.store(true);
    }

    virtual void loop() {
      if (!this->available_.load())
        throw fError(FURI_WRAP " !ystructure!! is closed\n", this->pattern()->toString().c_str());
      Option<Mail_p> mail = this->outbox_->pop_back();
      while (mail.has_value()) {
        LOG_STRUCTURE(TRACE, this, "Processing message %s for subscription %s\n",
                      mail.value()->second->toString().c_str(), mail.value()->first->toString().c_str());
        const Message_p message = mail.value()->second;
        if (!(message->retain && message->payload->is_noobj()))
          mail.value()->first->onRecv->apply(message->to_rec());
        mail = this->outbox_->pop_back();
      }
    }

    virtual void stop() {
      if (!this->available_.load())
        LOG_STRUCTURE(WARN, this, "!ystructure!! already closed");
      this->subscriptions_->clear();
      this->outbox_->clear(false);
      this->available_.store(false);
    }

    /////////////////////////////////////////////////

    virtual void recv_unsubscribe(const ID_p &source, const fURI_p &target) {
      if (!this->available_.load())
        LOG_STRUCTURE(ERROR, this, "!yunable to unsubscribe!! %s from %s\n", source->toString().c_str(),
                    target->toString().c_str());
      else
        this->mutex_.write<void *>([this, source,target]() {
          this->subscriptions_->erase(remove_if(this->subscriptions_->begin(), this->subscriptions_->end(),
                                                [source, target](const Subscription_p &sub) {
                                                  const bool removing =
                                                      sub->source.equals(*source) && (sub->pattern.matches(*target));
                                                  if (removing)
                                                    LOG_UNSUBSCRIBE(OK, *source, target);
                                                  return removing;
                                                }),
                                      this->subscriptions_->end());
          return nullptr;
        });
    }

    virtual void recv_subscription(const Subscription_p &subscription) {
      if (!this->available_.load()) {
        LOG_STRUCTURE(ERROR, this, "!yunable to receive!! %s\n", subscription->toString().c_str());
        return;
      }
      LOG_STRUCTURE(DEBUG, this, "!yreceived!! %s\n", subscription->toString().c_str());
      /////////////// DELETE EXISTING SUBSCRIPTION (IF EXISTS)
      this->recv_unsubscribe(id_p(subscription->source), p_p(subscription->pattern));
      /////////////// ADD NEW SUBSCRIPTION
      this->subscriptions_->push_back(subscription);
      LOG_SUBSCRIBE(OK, subscription);
      /////////////// HANDLE RETAINS MATCHING NEW SUBSCRIPTION
      if (!this->remote_retains_)
        this->publish_retained(subscription);
    }

    virtual void recv_publication(const Message_p &message) {
      if (!this->available_.load())
        throw fError("Structure " FURI_WRAP " is not available", this->pattern()->toString().c_str());
      ///////////////
      LOG_STRUCTURE(DEBUG, this, "!yreceived!! %s\n", message->toString().c_str());
      this->write(id_p(message->target), message->payload, id_p(message->source), message->retain);
      MESSAGE_INTERCEPT(message->source, message->target, message->payload, message->retain);
      LOG_PUBLISH(OK, *message);
    }


    virtual void remove(const ID_p &id, const ID_p &source) {
      this->write(id, noobj(), source, RETAIN_MESSAGE);
    }

    virtual void publish_retained(const Subscription_p &subscription) = 0;

    virtual Obj_p read(const fURI_p &furi, const ID_p &source) = 0;

    virtual Obj_p read(const fURI_p &furi) { return this->read(furi, id_p(FOS_DEFAULT_SOURCE_ID)); }

    virtual void write(const ID_p &id, const Obj_p &obj, const ID_p &source, bool retain) = 0;

    virtual void write(const ID_p &id, const Obj_p &obj, const bool retain) {
      this->write(id, obj, id_p(FOS_DEFAULT_SOURCE_ID), retain);
    }

  protected:
    ID_p resolve_id(const ID_p &key_id) const {
      return key_id->is_relative() ? id_p(this->pattern()->resolve(*key_id)) : key_id;
    }

    void check_availability(const string &function) const {
      if (!this->available())
        throw fError("Structure " FURI_WRAP " not available for %s", function.c_str());
    }

    Option<Obj_p> try_meta(const fURI_p &furi, const ID_p &) const {
      if (furi->has_query()) {
        if (strcmp(furi->query(), "sub") == 0)
          return {objs(this->get_subscription_objs(p_p(furi->query(nullptr))))};
        else
          return Obj::to_objs();
      }
      return {};
    }

    virtual void distribute_to_subscribers(const Message_p &message) {
      for (const Subscription_p &sub: *this->subscriptions_) {
        if (message->target.matches(sub->pattern))
          this->outbox_->push_back(share(Mail(sub, message)));
      }
    }

    bool has_equal_subscription_pattern(const fURI_p &topic, const ID_p &source = nullptr) {
      return this->mutex_.read<bool>([this,source,topic]() {
        for (const Subscription_p &sub: *this->subscriptions_) {
          if (source)
            if (!source->equals(sub->source))
              continue;
          if (topic->equals(sub->pattern))
            return true;
        }
        return false;
      });
    }

    bool has_matching_subscriptions(const fURI_p &topic, const ID_p &source = nullptr) {
      return this->mutex_.read<bool>([this,source,topic]() {
        for (const Subscription_p &sub: *this->subscriptions_) {
          if (source)
            if (!source->equals(sub->source))
              continue;
          if (topic->matches(sub->pattern))
            return true;
        }
        return false;
      });
    }

    List_p<Subscription_p> get_matching_subscriptions(const fURI_p &topic, const ID_p &source = nullptr) {
      return this->mutex_.read<List_p<Subscription_p>>([this,source,topic]() {
        List_p<Subscription_p> matches = share(List<Subscription_p>());
        for (const Subscription_p &sub: *this->subscriptions_) {
          if (source)
            if (!source->equals(sub->source))
              continue;
          if (topic->matches(sub->pattern) || sub->pattern.matches(*topic))
            matches->push_back(sub);
        }
        return matches;
      });
    }

    List<Obj_p> get_subscription_objs(const Pattern_p &pattern = p_p("#")) const {
      List<Obj_p> list;
      for (const Subscription_p &sub: *this->subscriptions_) {
        if (sub->pattern.matches(*pattern)) {
          const Rec_p sub_rec = Obj::to_rec({
                                              {uri(":source"), uri(sub->source)},
                                              {uri(":pattern"), uri(sub->pattern)},
                                              {uri(":qos"), jnt(static_cast<uint8_t>(sub->qos))},
                                              {uri(":on_recv"), sub->onRecv}
                                            }, id_p(REC_FURI->extend("sub")));
          list.push_back(sub_rec);
        }
      }
      return list;
    }
  };

  using Structure_p = ptr<Structure>;
} // namespace fhatos

#endif
