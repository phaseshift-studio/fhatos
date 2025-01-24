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
#include "../lang/obj.hpp"
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

    explicit Structure(const Rec_p &config) : Obj(*config),
                                              pattern_(p_p(config->rec_get("pattern")->uri_value())) {
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
        subscription->on_recv()->apply(message->payload(), Obj::to_rec({
                                         {"target", vri(message->target())},
                                         {"payload", message->payload()},
                                         {"retain", dool(message->retain())}
                                       }));
        mail = this->outbox_->pop_front();
      }
    }

    virtual void stop() {
      if(!this->available_.load())
        LOG_STRUCTURE(WARN, this, "!ystructure!! already stopped\n");
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
                sub->source()->equals(*source) && (sub->pattern()->matches(*target));
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
      this->recv_unsubscribe(subscription->source(), subscription->pattern());
      if(!subscription->on_recv()->is_noobj()) {
        /////////////// ADD NEW SUBSCRIPTION
        this->subscriptions_->push_back(subscription);
        LOG_SUBSCRIBE(OK, subscription);
        /////////////// HANDLE RETAINS MATCHING NEW SUBSCRIPTION
        this->publish_retained(subscription);
      }
    }

    virtual void publish_retained(const Subscription_p &subscription) {
      const IdObjPairs list = this->read_raw_pairs(subscription->pattern());
      for(const auto &[id, obj]: list) {
        if(!obj->is_noobj()) {
          if(id->matches(*subscription->pattern())) {
            FEED_WATCDOG();
            subscription->on_recv()->apply(obj, make_shared<Message>(id, obj,RETAIN));
          }
        }
      }
    }

    bool has(const fURI_p &furi) {
      return furi->is_node() ? !this->read(furi)->is_noobj() : !this->read_raw_pairs(furi_p(furi->extend("+"))).empty();
    }

    Obj_p read(const fURI &furi) {
      return this->read(furi_p(furi));
    }

    virtual Obj_p read(const fURI_p &furi) {
      if(!this->available_.load()) {
        LOG_STRUCTURE(ERROR, this, "!yunable to read!! %s\n", furi->toString().c_str());
        return noobj();
      }
      if(furi->has_query()) {
        const fURI_p furi_no_query = furi_p(furi->no_query());
        Objs_p ret = Obj::to_objs();
        if(furi->has_query("sub")) {
          ret = this->get_subscription_objs(furi_no_query);
          LOG_OBJ(DEBUG, this, "%i subscriptions received from %s\n",
                  ret->objs_value()->size(),
                  furi_no_query->toString().c_str());
        }
        if(furi->has_query("doc")) {
          if(const IdObjPairs doc = this->read_raw_pairs(furi); !doc.empty()) {
            for(const auto &pair: doc) {
              ret->add_obj(pair.second);
            }
          }
        }
        if(furi->query_value("type")) {
          const Objs_p objs = Obj::to_objs();
          objs->add_obj(this->read(furi_no_query));
          for(const Obj_p &obj: *objs->objs_value()) {
            const Obj_p type = ROUTER_READ(obj->tid_);
            ret->add_obj(type);
          }
        }
        if(furi->has_query("inst")) {
          const List<string> opcodes = furi->query_values("inst");
          const Rec_p insts = Obj::to_rec();
          const Objs_p objs = Obj::to_objs();
          objs->add_obj(ROUTER_READ(furi_p(furi_no_query->extend(":inst"))));
          for(const Obj_p &o: *objs->objs_value()) {
            if(!o->is_rec())
              throw fError("obj instructs must be records: %s", o->toString().c_str());
            insts->rec_merge(o->rec_value());
          }
          objs->objs_value()->clear();
          objs->add_obj(ROUTER_READ(furi_no_query));
          for(const Obj_p &o: *objs->objs_value()) {
            if(!FURI_OTYPE.count(*o->tid_))
              insts->rec_merge(ROUTER_READ(furi_p(o->tid_->query("inst")))->rec_value());
          }
          ret->add_obj(insts);
        }
        return ret;
      }
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////// READ BRANCH PATTERN/ID ///////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      const fURI_p temp = furi->is_branch()
                            ? furi_p(furi->extend("+").no_query())
                            : furi->has_query()
                                ? furi_p(furi->no_query())
                                : furi;
      const IdObjPairs matches = this->read_raw_pairs(temp);
      if(furi->is_branch()) {
        const Rec_p rec = Obj::to_rec();
        // BRANCH ID AND PATTERN
        for(const auto &[key, value]: matches) {
          rec->rec_set(vri(key), value, false);
        }
        return rec;
      } else {
        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////// READ NODE PATTERN/ID /////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        if(matches.empty()) {
          LOG(TRACE, "searching for base poly of: %s\n", furi->toString().c_str());
          if(const auto pair = this->locate_base_poly(furi_p(furi->retract())); pair.has_value()) {
            LOG(TRACE, "base poly found at %s: %s\n",
                pair->first->toString().c_str(),
                pair->second->toString().c_str());
            const fURI_p furi_subpath = id_p(furi->remove_subpath(pair->first->as_branch().toString(), true));
            const Poly_p poly_read = pair->second; //->clone();
            Obj_p read_obj = poly_read->poly_get(vri(furi_subpath));
            return read_obj;
          }
          return Obj::to_noobj();
        }
        if(!furi->is_pattern())
          return matches.front().second;
        else {
          const Objs_p objs = Obj::to_objs();
          for(const auto &o: matches) {
            objs->add_obj(o.second);
          }
          return objs;
        }
      }
    }

    Option<Pair<ID_p, Poly_p>> locate_base_poly(const fURI_p &furi) {
      fURI_p pc_furi = furi; // force it to be a node. good or bad?
      Obj_p obj = Obj::to_noobj();
      while(pc_furi->path_length() > 0) {
        obj = this->read(pc_furi);
        if(obj->is_poly()) break;
        pc_furi = furi_p(pc_furi->retract().as_node());
      }
      return obj->is_poly()
               ? Option<Pair<ID_p, Poly_p>>(Pair<ID_p, Poly_p>(id_p(*pc_furi), obj))
               : Option<Pair<ID_p, Poly_p>>();
    }

    virtual void write(const fURI_p &furi, const Obj_p &obj, const bool retain = RETAIN) {
      if(!this->available_.load()) {
        throw fError::create(this->vid_or_tid()->toString(), "!yunable to write!! %s\n", obj->toString().c_str());
      }
      if(furi->has_query()) {
        ////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// SUBSCRIBE ?sub={code|subscription|noobj} // noobj for unsubscribe
        ////////////////////////////////////////////////////////////////////////////////////////////////////////
        if(retain && furi->query_value("sub").has_value()) {
          const Pattern_p pattern = p_p(furi->no_query());
          if(obj->is_noobj()) {
            // unsubscribe
            this->recv_unsubscribe(
              (Process::current_process() ? Process::current_process()->vid_ : SCHEDULER_ID), pattern);
          } else if(obj->is_code()) {
            // bcode for on_recv
            this->recv_subscription(Subscription::create(
              Process::current_process() ? Process::current_process()->vid_ : SCHEDULER_ID, pattern, obj));
          } else if(obj->is_rec() && Compiler(false, false).type_check(obj.get(), SUBSCRIPTION_FURI)) {
            // complete sub[=>] record
            this->recv_subscription(make_shared<Subscription>(obj));
          }
          return;
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// APPLY ?apply={lhs|rhs} a.b or b.a
        ////////////////////////////////////////////////////////////////////////////////////////////////////////
        if(furi->has_query("apply")) {
          const string apply_position = furi->query_value("apply").value();
          const fURI_p furi_no_query = furi_p(furi->no_query()); //.to_ptr<>();
          const Obj_p old_obj = this->read(furi_no_query);
          Obj_p new_obj = nullptr;
          if("lhs" == apply_position) {
            LOG_OBJ(DEBUG, this, "apply?lhs: %s !m=>!! %s\n", obj->toString().c_str(),
                    old_obj->toString().c_str());
            new_obj = old_obj->apply(obj);
          } else if("rhs" == apply_position) {
            LOG_OBJ(DEBUG, this, "apply?rhs: %s !m=>!! %s\n", old_obj->toString().c_str(),
                    obj->toString().c_str());
            new_obj = obj->apply(old_obj);
          } else
            throw fError("invalid query value for !b?apply={!ylhs!b | !yrhs!b}!!: %s", apply_position.c_str());
          this->write(furi_no_query, new_obj, retain);
        }
      }

      //// WRITES
      const fURI_p new_furi = !furi->has_query() ? furi : furi_p(furi->no_query());
      if(new_furi->is_branch()) {
        // BRANCH (POLYS)
        if(obj->is_noobj()) {
          // nobj
          const IdObjPairs ids = this->read_raw_pairs(furi_p(new_furi->extend("+")));
          for(const auto &[key, value]: ids) {
            this->write_raw_pairs(key, obj, retain);
          }
        } // else if(obj->is_type() && obj->is_poly()) {
        // this->write_raw_pairs(id_p(*furi), obj, retain);
        /*}*/ else if(obj->is_rec()) {
          // rec
          const auto remaining = make_shared<RecMap<>>();
          for(const auto &[key, value]: *obj->rec_value()) {
            if(key->is_uri()) {
              // uri key
              this->write(id_p(key->uri_value()), value, retain);
              distribute_to_subscribers(Message::create(id_p(key->uri_value()), value, retain));
              // may be wrong, should be outside recurssion
            } else // non-uri key
              remaining->insert({key, value});
          }
          if(!remaining->empty()) {
            // non-uri keyed pairs written to /0
            this->write_raw_pairs(id_p(new_furi->extend("0")), Obj::to_rec(remaining), retain);
          }
        } else if(obj->is_lst()) {
          // lst /0,/1,/2 indexing
          const List_p<Obj_p> list = obj->lst_value();
          for(size_t i = 0; i < list->size(); i++) {
            this->write_raw_pairs(id_p(new_furi->extend(to_string(i))), list->at(i), retain);
          }
        } else {
          // BRANCH (MONOS)
          // monos written to /0
          this->write_raw_pairs(id_p(*new_furi), obj, retain);
        }
      } else {
        // NODE PATTERN
        if(new_furi->is_pattern()) {
          const IdObjPairs matches = this->read_raw_pairs(new_furi);
          for(const auto &[key, value]: matches) {
            this->write(key, obj, retain);
          }
        }
        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////// WRITE NODE ID ////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        else {
          LOG(TRACE, "searching for base poly of: %s\n", new_furi->toString().c_str());
          if(const auto pair = this->locate_base_poly(furi_p(new_furi->retract())); pair.has_value()) {
            LOG(TRACE, "base poly found at %s: %s\n",
                pair->first->toString().c_str(),
                pair->second->toString().c_str());
            const ID_p id_insert = id_p(new_furi->remove_subpath(pair->first->as_branch().toString(), true).to_node());
            const Poly_p poly_insert = pair->second; //->clone();
            poly_insert->poly_set(vri(id_insert), obj);
            distribute_to_subscribers(Message::create(id_p(*new_furi), obj, retain));
            LOG(TRACE, "base poly reinserted into structure at %s: %s\n",
                pair->first->toString().c_str(),
                poly_insert->toString().c_str());
            this->write(pair->first, poly_insert, retain); // NOTE: using write() so poly recursion happens
          } else {
            this->write_raw_pairs(id_p(*new_furi), obj, retain);
          }
        }
      }
      /*if(!retain) {
        // x --> y -< subscribers
        // non-retained writes 'pass through' (via application) any existing obj at that write furi location
        const Obj_p applicable_obj = this->read(furi); // get the obj that current exists at that furi
        if(applicable_obj->is_noobj()) {
          // no obj, pass the written obj through to subscribers (optimization assuming noobj is _ ?? bad??)
          this->write_raw_pairs(id_p(*furi), obj, retain);
        } else if(applicable_obj->is_code()) {
          // code: pass the output of applying the written obj to code to subscribers
          // TODO: InstArgs should be Rec_p (with _0 being index to simulate Lst)
          const Obj_p result = applicable_obj->apply(obj);
          //ObjHelper::replace_from_obj({obj}, applicable_obj)->apply(obj);
          //ObjHelper::apply_lhs_args(applicable_obj, obj);
          // Options::singleton()->processor<Obj, BCode, Obj>(obj, rewritten_bcode);
          this->write_raw_pairs(id_p(*furi), result, retain);
        } else
        // any other obj, apply it (which for monos, will typically result in providing subscribers with the already existing obj)
          this->write_raw_pairs(id_p(*furi), applicable_obj->apply(obj), retain);
      }*/
    }

    /////////////////////////////////////////////////////////////////////////////
  public:
    virtual void write_raw_pairs(const ID_p &id, const Obj_p &obj, bool retain) = 0;

    virtual IdObjPairs read_raw_pairs(const fURI_p &match) = 0;

  protected:
    static Obj_p strip_value_id(const Obj_p &obj) {
      return nullptr == obj->vid_ ? obj : Obj::create(obj->value_, obj->o_type(), obj->tid_, nullptr);
    }

    void check_availability(const string &function) const {
      if(!this->available())
        throw fError("structure " FURI_WRAP " not available for %s", function.c_str());
    }

    virtual void distribute_to_subscribers(const Message_p &message) {
      if(!message->payload()->is_noobj()) {
        // LOG_OBJ(DEBUG, this, "distributing message %s to subscribers [size:%i]\n", message->toString().c_str(),
        //         this->subscriptions_->size());
        this->subscriptions_->forEach([this,message](const Subscription_p &subscription) {
          if(message->target()->matches(*subscription->pattern()))
            this->outbox_->push_back(mail_p(subscription, message));
        });
      }
    }

    bool has_equal_subscription_pattern(const fURI_p &topic, const ID_p &source = nullptr) const {
      return this->subscriptions_->exists([source,topic](const Subscription_p &sub) {
        if(source && !source->equals(*sub->source()))
          return false;
        if(topic->equals(*sub->pattern()))
          return true;
        return false;
      });
    }

    List_p<Subscription_p> get_matching_subscriptions(const fURI_p &topic, const ID_p &source = nullptr) const {
      const List_p<Subscription_p> matches = make_shared<List<Subscription_p>>();
      this->subscriptions_->forEach([topic,source,matches](const Subscription_p &subscription) {
        if(!(source && !source->equals(*subscription->source())) && topic->bimatches(*subscription->pattern()))
          matches->push_back(subscription);
      });
      return matches;
    }

    Objs_p get_subscription_objs(const fURI_p &pattern = p_p("#")) const {
      const Objs_p objs = Obj::to_objs();
      this->subscriptions_->forEach([pattern,objs](const Subscription_p &subscription) {
        if(subscription->pattern()->bimatches(*pattern)) {
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
