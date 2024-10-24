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
#include <process/process.hpp>
#include <structure/pubsub.hpp>
#include <util/enums.hpp>
#include <util/mutex_deque.hpp>
#include <util/mutex_rw.hpp>

#define FOS_TRY_META                                                                                                   \
  const Option<Obj_p> meta = this->try_meta(furi);                                                                     \
  if (meta.has_value())                                                                                                \
    return meta.value();


namespace fhatos {
  using ReadRawResult = List<Pair<ID_p, Obj_p>>;
  using ReadRawResult_p = List_p<Pair<ID_p, Obj_p>>;

  const Pattern_p HEAP_FURI = p_p(REC_FURI->resolve("heap"));
  const Pattern_p COMPUTED_FURI = p_p(REC_FURI->resolve("computed"));
  const Pattern_p MQTT_FURI = p_p(REC_FURI->resolve("mqtt"));
  const Pattern_p DISK_FURI = p_p(REC_FURI->resolve("disk"));
  const Pattern_p BLE_FURI = p_p(REC_FURI->resolve("ble"));

  class Router;

  enum class SType { COMPUTED, HEAP, MQTT, DISK, BLE };

  static const Enums<SType> StructureTypes =
      Enums<SType>({{SType::COMPUTED, "computed"}, {SType::HEAP, "heap"}, {SType::MQTT, "mqtt"}, {SType::DISK, "disk"}, {SType::BLE,"ble"}});

  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class Structure : public Patterned {
  protected:
    ptr<MutexDeque<Mail_p>> outbox_ = std::make_shared<MutexDeque<Mail_p>>();
    ptr<List<Subscription_p>> subscriptions_ = std::make_shared<List<Subscription_p>>();
    MutexRW<> mutex_ = MutexRW<>("structure_mutex");
    std::atomic_bool available_ = std::atomic_bool(false);

  public:
    const SType stype;

    explicit Structure(const Pattern &pattern, const SType stype) : Patterned(p_p(pattern)), stype(stype) {
    }

    [[nodiscard]] bool available() const { return this->available_.load(); }

    virtual void setup() {
      if (this->available_.load()) {
        LOG_STRUCTURE(WARN, this, "!ystructure!! already available\n");
        return;
      }
      this->available_.store(true);
    }

    virtual void loop() {
      if (!this->available_.load())
        throw fError(FURI_WRAP " !ystructure!! is closed", this->pattern()->toString().c_str());
      Option<Mail_p> mail = this->outbox_->pop_front();
      while (mail.has_value()) {
        LOG_STRUCTURE(TRACE, this, "Processing message %s for subscription %s\n",
                      mail.value()->second->toString().c_str(), mail.value()->first->toString().c_str());
        const Message_p message = mail.value()->second;
        // if (!(message->retain && message->payload->is_noobj()))
        const Subscription_p subscription = mail.value()->first;
        subscription->on_recv->apply(message->to_rec());
        mail = this->outbox_->pop_front();
      }
    }

    virtual void stop() {
      if (!this->available_.load())
        LOG_STRUCTURE(WARN, this, "!ystructure!! already unavailable\n");
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
      if (!subscription->on_recv->is_noobj()) {
        /////////////// ADD NEW SUBSCRIPTION
        this->subscriptions_->push_back(subscription);
        LOG_SUBSCRIBE(OK, subscription);
        /////////////// HANDLE RETAINS MATCHING NEW SUBSCRIPTION
        this->publish_retained(subscription);
      }
    }

    virtual void publish_retained(const Subscription_p &subscription) {
      const List<Pair<ID_p, Obj_p>> list = this->read_raw_pairs(furi_p(subscription->pattern));
      for (const auto &[id, obj]: list) {
        if (!obj->is_noobj()) {
          if (id->matches(subscription->pattern)) {
            subscription->on_recv->apply(Message(*id, obj,RETAIN_MESSAGE).to_rec());
          }
        }
      }
    }

    virtual Obj_p read(const fURI_p &furi) {
      if (!this->available_.load()) {
        LOG_STRUCTURE(ERROR, this, "!yunable to read!! %s\n", furi->toString().c_str());
        return noobj();
      }
      if (furi->has_query() && string(furi->query()) == "sub") {
        if (string(furi->query()) == "sub") {
          const Objs_p subs = this->get_subscription_objs(p_p(furi->query("")));
          return subs;
        } else if (furi->query_value("doc").has_value()) {
          return noobj();
          // const Str_p doc = this->getd
        }
      } else {
        const fURI_p temp = furi->is_branch() ? furi_p(furi->extend("+")) : furi;
        const List<Pair<ID_p, Obj_p>> matches = this->read_raw_pairs(temp);
        if (furi->is_branch()) {
          const Rec_p rec = Obj::to_rec();
          // BRANCH ID AND PATTERN
          for (const auto &[key, value]: matches) {
            rec->rec_set(vri(key), value);
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
            if (matches.empty())
              return noobj();
            const Obj_p o = matches.begin()->second;
            return o;
          }
        }
      }
      return noobj();
    }

    virtual void write(const fURI_p &furi, const Obj_p &obj, const bool retain = RETAIN_MESSAGE) {
      if (!this->available_.load()) {
        LOG_STRUCTURE(ERROR, this, "!yunable to write!! %s\n", obj->toString().c_str());
        return;
      }
      if (retain) {
        if (furi->has_query() && furi->query_value("sub").has_value()) {
          //// SUBSCRIBE
          if (furi->query_value("sub").has_value()) {
            const Pattern_p pattern = p_p(furi->query(""));
            if (obj->is_noobj()) {
              // unsubscribe
              this->recv_unsubscribe(Process::current_process()->id(), pattern);
            } else if (obj->is_bcode()) {
              // bcode for on_recv
              this->recv_subscription(subscription_p(*Process::current_process()->id(), *pattern, obj));
            } else if (obj->is_rec()) {
              // complete sub[=>] record
              this->recv_subscription(from_subscription_obj(obj));
            }
          }
        } else {
          //// WRITES
          if (furi->is_branch()) {
            // BRANCH (POLYS)
            if (obj->is_noobj()) {
              // nobj
              const List<Pair<ID_p, Obj_p>> ids = this->read_raw_pairs(furi_p(furi->extend("+")));
              for (const auto &[key, value]: ids) {
                this->write_raw_pairs(key, obj, retain);
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
                this->write_raw_pairs(id_p(furi->extend("0")), Obj::to_rec(remaining), retain);
              }
            } else if (obj->is_lst()) {
              // lst /0,/1,/2 indexing
              const List_p<Obj_p> list = obj->lst_value();
              for (size_t i = 0; i < list->size(); i++) {
                this->write_raw_pairs(id_p(furi->extend(to_string(i))), list->at(i), retain);
              }
            } else {
              // BRANCH (MONOS)
              // monos written to /0
              this->write_raw_pairs(id_p(*furi), obj, retain);
            }
          } else {
            // NODE PATTERN
            if (furi->is_pattern()) {
              const List<Pair<ID_p, Obj_p>> matches = this->read_raw_pairs(furi_p(*furi));
              for (const auto &[key, value]: matches) {
                this->write_raw_pairs(key, obj, retain);
              }
            }
            // NODE ID
            else {
              this->write_raw_pairs(id_p(*furi), obj, retain);
            }
          }
        }
      } else {
        this->write_raw_pairs(id_p(*furi), obj, retain);
      }
    }

    /////////////////////////////////////////////////////////////////////////////
  public:
    virtual void write_raw_pairs(const ID_p &id, const Obj_p &obj, bool retain) = 0;

    virtual List<Pair<ID_p, Obj_p>> read_raw_pairs(const fURI_p &match) = 0;

  protected:
    static Obj_p strip_value_id(const Obj_p &obj) {
      return nullptr == obj->id() ? obj : make_shared<Obj>(obj->_value, obj->type(), nullptr);
    }

    void check_availability(const string &function) const {
      if (!this->available())
        throw fError("Structure " FURI_WRAP " not available for %s", function.c_str());
    }

    virtual void distribute_to_subscribers(const Message_p &message) {
      for (const Subscription_p &sub: *this->subscriptions_) {
        if (message->target.matches(sub->pattern))
          this->outbox_->push_back(mail_p(sub, message));
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
        const List_p<Subscription_p> matches = share(List<Subscription_p>());
        for (const Subscription_p &sub: *this->subscriptions_) {
          if (source && !source->equals(sub->source))
            continue;
          if (topic->bimatches(sub->pattern))
            matches->push_back(sub);
        }
        return matches;
      });
    }

    Objs_p get_subscription_objs(const Pattern_p &pattern = p_p("#")) const {
      Objs_p objs = Obj::to_objs();
      for (const Subscription_p &sub: *this->subscriptions_) {
        if (sub->pattern.bimatches(*pattern)) {
          const Rec_p sub_rec = Obj::to_rec({{vri(":source"), vri(sub->source)},
                                              {vri(":pattern"), vri(sub->pattern)},
                                              {vri(":on_recv"), sub->on_recv}},
                                            id_p(REC_FURI->extend("sub")));
          objs->add_obj(sub_rec);
        }
      }
      return objs;
    }

    static Subscription_p from_subscription_obj(const Rec_p &rec) {
      return subscription_p(rec->rec_get(vri(":source"))->uri_value(),
                            rec->rec_get(vri(":pattern"))->uri_value(),
                            rec->rec_get(vri(":on_recv")));
    }
  };

  using Structure_p = ptr<Structure>;
} // namespace fhatos

#endif
