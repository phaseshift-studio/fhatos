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

#include "../fhatos.hpp"
#include "../language/obj.hpp"
#include "../process/process.hpp"
#include "pubsub.hpp"
#include "../util/enums.hpp"
#include "../util/mutex_deque.hpp"

#define FOS_TRY_META                                                                                                   \
  const Option<Obj_p> meta = this->try_meta(furi);                                                                     \
  if (meta.has_value())                                                                                                \
    return meta.value();


namespace fhatos {
  using IdObjPairs = List<Pair<ID_p, Obj_p>>;
  using IdObjPairs_p = List_p<Pair<ID_p, Obj_p>>;

  const ID_p HEAP_FURI = id_p(FOS_SCHEME "/heap");
  const ID_p COMPUTED_FURI = id_p(FOS_SCHEME "/computed");
  const ID_p MQTT_FURI = id_p(FOS_SCHEME "/mqtt");
  const ID_p DISK_FURI = id_p(FOS_SCHEME "/disk");
  const ID_p BLE_FURI = id_p(FOS_SCHEME "/ble");

  class Router;

  /////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class Structure : public Rec {
  protected:
    ptr<MutexDeque<Mail_p>> outbox_ = std::make_shared<MutexDeque<Mail_p>>();
    ptr<MutexDeque<Subscription_p>> subscriptions_ = std::make_shared<MutexDeque<Subscription_p>>();
    std::atomic_bool available_ = std::atomic_bool(false);

  public:
    const Pattern_p pattern_;

    explicit Structure(const Pattern &pattern, const ID &value_id) : Rec(Obj::RecMap<>(), OType::REC, REC_FURI,
                                                                         nullptr), pattern_(p_p(pattern)) {
    }

    explicit Structure(const Rec_p &structure_rec) : Obj(*structure_rec->rec_merge(rmap({{
                                                       id_p(":detach"), Obj::to_bcode(
                                                         [this](const Obj_p &) {
                                                           this->stop();
                                                           return noobj();
                                                         }, StringHelper::cxx_f_metadata(__FILE__,__LINE__))
                                                     }}))),
                                                     pattern_(p_p(
                                                       structure_rec->rec_get(vri("pattern"))->uri_value())) {
    }

    [[nodiscard]] Pattern_p pattern() const {
      return this->pattern_;
    }

    [[nodiscard]] bool available() const { return this->available_.load(); }

    virtual void setup() {
      if(this->available_.load()) {
        LOG_STRUCTURE(WARN, this, "!ystructure!! already available\n");
        return;
      }
      this->available_.store(true);
    }

    virtual void loop() {
      if(!this->available_.load())
        throw fError(FURI_WRAP " !ystructure!! is closed", this->pattern()->toString().c_str());
      Option<Mail_p> mail = this->outbox_->pop_front();
      while(mail.has_value()) {
        FEED_WATCDOG();
        LOG_STRUCTURE(TRACE, this, "processing message %s for subscription %s\n",
                      mail.value()->second->toString().c_str(), mail.value()->first->toString().c_str());
        const Message_p message = mail.value()->second;
        const Subscription_p subscription = mail.value()->first;
        subscription->process_message(message);
        mail = this->outbox_->pop_front();
      }
    }

    virtual void stop() {
      if(!this->available_.load())
        LOG_STRUCTURE(WARN, this, "!ystructure!! already unavailable\n");
      this->subscriptions_->clear();
      this->outbox_->clear(false);
      this->available_ = false;
    }

    /////////////////////////////////////////////////

    virtual void recv_unsubscribe(const ID_p &source, const fURI_p &target) {
      if(!this->available_.load())
        LOG_STRUCTURE(ERROR, this, "!yunable to unsubscribe!! %s from %s\n", source->toString().c_str(),
                    target->toString().c_str());
      else {
        this->subscriptions_->remove_if(
          [source, target](const Subscription_p &sub) {
            const bool removing =
                sub->source().equals(*source) && (sub->pattern().matches(*target));
            if(removing)
              LOG_UNSUBSCRIBE(OK, source, target);
            return removing;
          });
      }
    }

    virtual void recv_subscription(const Subscription_p &subscription) {
      if(!this->available_.load()) {
        LOG_STRUCTURE(ERROR, this, "!yunable to receive!! %s\n", subscription->toString().c_str());
        return;
      }
      LOG_STRUCTURE(DEBUG, this, "!yreceived!! %s\n", subscription->toString().c_str());
      /////////////// DELETE EXISTING SUBSCRIPTION (IF EXISTS)
      this->recv_unsubscribe(id_p(subscription->source()), p_p(subscription->pattern()));
      if(!subscription->on_recv()->is_noobj()) {
        /////////////// ADD NEW SUBSCRIPTION
        this->subscriptions_->push_back(subscription);
        LOG_SUBSCRIBE(OK, subscription);
        /////////////// HANDLE RETAINS MATCHING NEW SUBSCRIPTION
        this->publish_retained(subscription);
      }
    }

    virtual void publish_retained(const Subscription_p &subscription) {
      const IdObjPairs list = this->read_raw_pairs(furi_p(subscription->pattern()));
      for(const auto &[id, obj]: list) {
        if(!obj->is_noobj()) {
          if(id->matches(subscription->pattern())) {
            FEED_WATCDOG();
            subscription->process_message(make_shared<Message>(*id, obj,RETAIN));
          }
        }
      }
    }

    bool has(const fURI_p &furi) {
      return furi->is_node() ? !this->read(furi)->is_noobj() : !this->read_raw_pairs(furi_p(furi->extend("+"))).empty();
    }

    virtual Obj_p read(const fURI_p &furi) {
      if(!this->available_.load()) {
        LOG_STRUCTURE(ERROR, this, "!yunable to read!! %s\n", furi->toString().c_str());
        return noobj();
      }
      if(furi->has_query()) {
        const fURI_p query_less_furi = furi_p(furi->query(""));
        const Objs_p ret = Obj::to_objs();
        if(furi->has_query("sub")) {
          ret->add_obj(this->get_subscription_objs(query_less_furi));
        } else if(furi->has_query("doc")) {
          if(const IdObjPairs doc = this->read_raw_pairs(furi); !doc.empty()) {
            for(const auto &pair: doc) {
              ret->add_obj(pair.second);
            }
          }
        } else if(furi->query_value("type")) {
          const Objs_p objs = Obj::to_objs();
          objs->add_obj(this->read(query_less_furi));
          for(const Obj_p &obj: *objs->objs_value()) {
            const Obj_p type = ROUTER_READ(obj->tid());
            ret->add_obj(type);
          }
        } else if(furi->has_query("inst")) {
          const List<string> opcodes = furi->query_values("inst");
          const Rec_p insts = Obj::to_rec();
          const Objs_p objs = Obj::to_objs();
          objs->add_obj(ROUTER_READ(furi_p(query_less_furi->extend(":inst"))));
          for(const Obj_p &o: *objs->objs_value()) {
            if(!o->is_rec())
              throw fError("obj instructs must be records: %s", o->toString().c_str());
            insts->rec_merge(o->rec_value());
          }
          objs->objs_value()->clear();
          objs->add_obj(ROUTER_READ(query_less_furi));
          for(const Obj_p &o: *objs->objs_value()) {
            if(!FURI_OTYPE.count(*o->tid()))
              insts->rec_merge(ROUTER_READ(furi_p(o->tid()->query("inst")))->rec_value());
          }
          ret->add_obj(insts);
        }
        return ret;
      }
      //////////////////////////////////////////////////////////////////////////
      const fURI_p temp = furi->is_branch() ? furi_p(furi->extend("+")) : furi;
      const IdObjPairs matches = this->read_raw_pairs(temp);
      if(furi->is_branch()) {
        const Rec_p rec = Obj::to_rec();
        // BRANCH ID AND PATTERN
        for(const auto &[key, value]: matches) {
          rec->rec_set(vri(key), value, false);
        }
        return rec;
      } else {
        // NODE PATTERN
        if(furi->is_pattern()) {
          const Objs_p objs = Obj::to_objs();
          for(const auto &[key, value]: matches) {
            objs->add_obj(value);
          }
          return objs;
        }
        // NODE ID
        else {
          if(matches.empty()) {
            if(furi->path_length() > 0) {
              // recurse backwards to find a root poly that has respective furi path
              const Obj_p maybe_poly = this->read(furi_p(furi->retract().as_node()));
              if(maybe_poly->is_rec())
                return maybe_poly->rec_get(vri(furi->name()));
              if(maybe_poly->is_lst() && StringHelper::is_integer(furi->name()))
                return maybe_poly->lst_get(jnt(stoi(furi->name())));
            }
            return noobj();
          }
          return matches.front().second;
        }
      }
    }

    virtual void write(const fURI_p &furi, const Obj_p &obj, const bool retain = RETAIN) {
      if(!this->available_.load()) {
        LOG_STRUCTURE(ERROR, this, "!yunable to write!! %s\n", obj->toString().c_str());
        return;
      }
      if(retain) {
        // x -> y
        if(furi->has_query()) {
          // x -> ?meta (writing to meta furis)
          //// SUBSCRIBE
          if(furi->query_value("sub").has_value()) {
            const Pattern_p pattern = p_p(furi->query(""));
            if(obj->is_noobj()) {
              // unsubscribe
              this->recv_unsubscribe(
                (Process::current_process() ? Process::current_process()->vid() : id_p("scheduler")), pattern);
            } else if(obj->is_bcode()) {
              // bcode for on_recv
              this->recv_subscription(Subscription::create(
                Process::current_process() ? *Process::current_process()->vid() : ID("scheduler"), *pattern, obj));
            } else if(obj->is_rec() && TYPE_CHECKER(obj.get(), SUBSCRIPTION_FURI, false)) {
              // complete sub[=>] record
              this->recv_subscription(make_shared<Subscription>(obj));
            }
            return;
          }
        }
        //// WRITES
        if(furi->is_branch()) {
          // BRANCH (POLYS)
          if(obj->is_noobj()) {
            // nobj
            const IdObjPairs ids = this->read_raw_pairs(furi_p(furi->extend("+")));
            for(const auto &[key, value]: ids) {
              this->write_raw_pairs(key, obj, retain);
            }
          } else if(obj->is_type() && obj->is_poly()) {
            this->write_raw_pairs(id_p(*furi), obj, retain);
          } else if(obj->is_rec()) {
            // rec
            const auto remaining = share(Obj::RecMap<>());
            for(const auto &[key, value]: *obj->rec_value()) {
              if(key->is_uri()) {
                // uri key
                this->write(id_p(key->uri_value()), value, retain);
                distribute_to_subscribers(Message::create(ID(key->uri_value()), value, retain));
                // may be wrong, should be outside recurssion
              } else // non-uri key
                remaining->insert({key, value});
            }
            if(!remaining->empty()) {
              // non-uri keyed pairs written to /0
              this->write_raw_pairs(id_p(furi->extend("0")), Obj::to_rec(remaining), retain);
            }
          } else if(obj->is_lst()) {
            // lst /0,/1,/2 indexing
            const List_p<Obj_p> list = obj->lst_value();
            for(size_t i = 0; i < list->size(); i++) {
              this->write_raw_pairs(id_p(furi->extend(to_string(i))), list->at(i), retain);
            }
          } else {
            // BRANCH (MONOS)
            // monos written to /0
            this->write_raw_pairs(id_p(*furi), obj, retain);
          }
        } else {
          // NODE PATTERN
          if(furi->is_pattern()) {
            const IdObjPairs matches = this->read_raw_pairs(furi_p(*furi));
            for(const auto &[key, value]: matches) {
              this->write_raw_pairs(key, obj, retain);
            }
          }
          // NODE ID
          else {
            /*const IdObjPairs_p matches = this->read_raw_pairs(furi);
            if (matches->empty()) {
              if (furi->path_length() > 1) {
                const Rec_p rec = this->read(furi_p(furi->retract()));
                if (rec->is_rec()) {
                  rec->rec_set(furi_p(furi->name()), obj);
                  write_raw_pairs(id_p(*furi), rec, retain);
                  return;
                }
              }
            }*/
            this->write_raw_pairs(id_p(*furi), obj, retain);
          }
        }
      } else {
        // x --> y -< subscribers
        // non-retained writes 'pass through' (via application) any existing obj at that write furi location
        const Obj_p applicable_obj = this->read(furi); // get the obj that current exists at that furi
        if(applicable_obj->is_noobj()) {
          // no obj, pass the written obj through to subscribers (optimization assuming noobj is _ ?? bad??)
          this->write_raw_pairs(id_p(*furi), obj, retain);
        } else if(applicable_obj->is_bcode()) {
          // bcode, pass the output of applying the written obj to bcode to subscribers
          //const BCode_p rewritten_bcode = ;
          // TODO: InstArgs should be Rec_p (with _0 being index to simulate Lst)
          const Obj_p result = applicable_obj->apply(obj);
          //ObjHelper::replace_from_obj({obj}, applicable_obj)->apply(obj);
          //ObjHelper::apply_lhs_args(applicable_obj, obj);
          // Options::singleton()->processor<Obj, BCode, Obj>(obj, rewritten_bcode);
          this->write_raw_pairs(id_p(*furi), result, retain);
        } else
        // any other obj, apply it (which for monos, will typically result in providing subscribers with the already existing obj)
          this->write_raw_pairs(id_p(*furi), applicable_obj->apply(obj), retain);
      }
    }

    /////////////////////////////////////////////////////////////////////////////
  public:
    virtual void write_raw_pairs(const ID_p &id, const Obj_p &obj, bool retain) = 0;

    virtual IdObjPairs read_raw_pairs(const fURI_p &match) = 0;

  protected:
    static Obj_p strip_value_id(const Obj_p &obj) {
      return nullptr == obj->vid() ? obj : Obj::create(obj->value_, obj->o_type(), obj->tid(), nullptr);
    }

    void check_availability(const string &function) const {
      if(!this->available())
        throw fError("structure " FURI_WRAP " not available for %s", function.c_str());
    }

    virtual void distribute_to_subscribers(const Message_p &message) {
      this->subscriptions_->forEach([this,message](const Subscription_p &subscription) {
        if(message->target().matches(subscription->pattern()))
          this->outbox_->push_back(mail_p(subscription, message));
      });
    }

    bool has_equal_subscription_pattern(const fURI_p &topic, const ID_p &source = nullptr) const {
      return this->subscriptions_->exists([source,topic](const Subscription_p &sub) {
        if(source && !source->equals(sub->source()))
          return false;
        if(topic->equals(sub->pattern()))
          return true;
        return false;
      });
    }

    List_p<Subscription_p> get_matching_subscriptions(const fURI_p &topic, const ID_p &source = nullptr) const {
      const List_p<Subscription_p> matches = share(List<Subscription_p>());
      this->subscriptions_->forEach([topic,source,matches](const Subscription_p &subscription) {
        if(!(source && !source->equals(subscription->source())) && topic->bimatches(subscription->pattern()))
          matches->push_back(subscription);
      });
      return matches;
    }

    Objs_p get_subscription_objs(const fURI_p &pattern = p_p("#")) const {
      const Objs_p objs = Obj::to_objs();
      this->subscriptions_->forEach([pattern,objs](const Subscription_p &subscription) {
        if(subscription->pattern().bimatches(*pattern)) {
          objs->add_obj(subscription);
        }
      });
      return objs;
    }
  };

  using Structure_p = ptr<Structure>;
  using Structure_up = unique_ptr<Structure>;
} // namespace fhatos

#endif
