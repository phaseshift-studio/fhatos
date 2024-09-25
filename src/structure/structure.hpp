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

#define FOS_TRY_META                                                                                                   \
  const Option<Obj_p> meta = this->try_meta(furi);                                                                     \
  if (meta.has_value())                                                                                                \
    return meta.value();


namespace fhatos {
  class Router;

  enum class SType { EPHEMERAL, VARIABLES, DATABASE, HARDWARE, NETWORKED };

  static const Enums<SType> StructureTypes = Enums<SType>({{SType::EPHEMERAL, "ephemeral"},
    {SType::VARIABLES, "variables"},
    {SType::DATABASE, "database"},
    {SType::HARDWARE, "hardware"},
    {SType::NETWORKED, "networked"}});

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

    explicit Structure(const Pattern &pattern, const SType stype) : Patterned(p_p(pattern)), stype(stype) {
    }

    [[nodiscard]] bool available() const { return this->available_.load(); }

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
      Option<Mail_p> mail = this->outbox_->pop_front();
      while (mail.has_value()) {
        LOG_STRUCTURE(TRACE, this, "Processing message %s for subscription %s\n",
                      mail.value()->second->toString().c_str(), mail.value()->first->toString().c_str());
        const Message_p message = mail.value()->second;
        // if (!(message->retain && message->payload->is_noobj()))
        mail.value()->first->on_recv->apply(message->to_rec());
        mail = this->outbox_->pop_front();
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
        this->mutex_.write<void *>([this, source, target]() {
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
      this->write(id_p(message->target), message->payload, message->retain);
      LOG_PUBLISH(OK, *message);
    }

    virtual void remove(const ID_p &id) { this->write(id, noobj(), RETAIN_MESSAGE); }

    virtual void publish_retained(const Subscription_p &subscription) {
      const List<Pair<ID_p, Obj_p>> list = this->read_raw_pairs(furi_p(subscription->pattern));
      for (const auto &[id, obj]: list) {
        if (id->matches(subscription->pattern)) {
          if (!obj->is_noobj()) {
            subscription->on_recv->apply(Message{.target = *id, .payload = obj, .retain = RETAIN_MESSAGE}.to_rec());
          }
        }
      }
    }

    virtual Obj_p read(const fURI_p &furi) {
      const fURI_p temp = furi->is_branch() ? furi_p(furi->extend("+")) : furi;
      const List<Pair<ID_p, Obj_p>> matches = this->read_raw_pairs(temp);
      if (furi->is_branch()) {
        const Rec_p rec = Obj::to_rec();
        // BRANCH ID AND PATTERN
        for (const auto &[key, value]: matches) {
          rec->rec_set(uri(key), value);
        }
        return rec;
      } else {
        // NODE PATTERN
        if (furi->is_pattern()) {
          const Objs_p objs = Obj::to_objs();
          for (const auto &[key, value]: matches) {
            objs->add_obj(value);
          }
          return objs;
        }
        // NODE ID
        else {
          return matches.empty() ? noobj() : matches.begin()->second;
        }
      }
    }

    virtual void write(const fURI_p &furi, const Obj_p &obj, const bool retain) {
      if (retain) {
        if (furi->is_branch()) {
          // BRANCH (POLYS)
          if (obj->is_noobj()) {
            // nobj
            const List<Pair<ID_p, Obj_p>> ids = this->read_raw_pairs(furi_p(furi->extend("+")));
            for (const auto &id2: ids) {
              this->write_raw_pairs(id2.first, obj);
              distribute_to_subscribers(message_p(*id2.first, obj, retain));
            }
          } else if (obj->is_rec()) {
            // rec
            const auto remaining = share(Obj::RecMap<>());
            for (const auto &[key, value]: *obj->rec_value()) {
              if (key->is_uri()) {
                // uri key
                this->write(id_p(key->uri_value()), value, retain);
                distribute_to_subscribers(message_p(ID(key->uri_value()), value, retain));
              } else // non-uri key
                remaining->insert({key, value});
            }
            if (!remaining->empty()) {
              // non-uri keyed pairs written to /0
              this->write_raw_pairs(id_p(furi->extend("0")), Obj::to_rec(remaining));
              distribute_to_subscribers(
                message_p(ID(furi->extend("0")), Obj::to_rec(remaining), retain));
            }
          } else if (obj->is_lst()) {
            // lst /0,/1,/2 indexing
            const List_p<Obj_p> list = obj->lst_value();
            for (size_t i = 0; i < list->size(); i++) {
              this->write_raw_pairs(id_p(furi->extend(to_string(i))), list->at(i));
              distribute_to_subscribers(message_p(furi->extend(to_string(i)), list->at(i), retain));
            }
          } else {
            // BRANCH (MONOS)
            // monos written to /0
            this->write_raw_pairs(id_p(furi->extend("0")), obj);
            distribute_to_subscribers(message_p(ID(furi->extend("0")), obj, retain));
          }
        } else {
          // NODE PATTERN
          if (furi->is_pattern()) {
            const List<Pair<ID_p, Obj_p>> matches = this->read_raw_pairs(furi_p(*furi));
            for (const auto &id: matches) {
              this->write_raw_pairs(id.first, obj);
              distribute_to_subscribers(message_p(*id.first, obj, retain));
            }
          }
          // NODE ID
          else {
            this->write_raw_pairs(id_p(*furi), obj);
            distribute_to_subscribers(message_p(ID(*furi), obj, retain));
          }
        }
      } else {
        distribute_to_subscribers(message_p(ID(*furi), obj, retain));
      }
    }

    /////////////////////////////////////////////////////////////////////////////
  protected:
    virtual void write_raw_pairs(const ID_p &id, const Obj_p &obj) = 0;

    virtual List<Pair<ID_p, Obj_p>> read_raw_pairs(const fURI_p &match) = 0;

    void check_availability(const string &function) const {
      if (!this->available())
        throw fError("Structure " FURI_WRAP " not available for %s", function.c_str());
    }

    Option<Obj_p> try_meta(const fURI_p &furi) const {
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
      return this->mutex_.read<bool>([this, source, topic]() {
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

    List_p<Subscription_p> get_matching_subscriptions(const fURI_p &topic, const ID_p &source = nullptr) {
      return this->mutex_.read<List_p<Subscription_p>>([this, source, topic]() {
        List_p<Subscription_p> matches = share(List<Subscription_p>());
        for (const Subscription_p &sub: *this->subscriptions_) {
          if (source)
            if (!source->equals(sub->source))
              continue;
          if (topic->matches(sub->pattern))
            matches->push_back(sub);
        }
        return matches;
      });
    }

    List<Obj_p> get_subscription_objs(const Pattern_p &pattern = p_p("#")) const {
      List<Obj_p> list;
      for (const Subscription_p &sub: *this->subscriptions_) {
        if (sub->pattern.matches(*pattern)) {
          const Rec_p sub_rec = Obj::to_rec({{uri(":source"), uri(sub->source)},
                                              {uri(":pattern"), uri(sub->pattern)},
                                              {uri(":qos"), jnt(static_cast<uint8_t>(sub->qos))},
                                              {uri(":on_recv"), sub->on_recv}},
                                            id_p(REC_FURI->extend("sub")));
          list.push_back(sub_rec);
        }
      }
      return list;
    }
  };

  using Structure_p = ptr<Structure>;
} // namespace fhatos

#endif
