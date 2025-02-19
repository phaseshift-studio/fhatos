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
#include "../model/fos/sys/thread/fmutex.hpp"

#define FOS_TRY_META                                                                                                   \
  const Option<Obj_p> meta = this->try_meta(furi);                                                                     \
  if (meta.has_value())                                                                                                \
    return meta.value();


namespace fhatos {
  using IdObjPairs = List<Pair<ID, Obj_p>>;
  using IdObjPairs_p = List_p<Pair<ID, Obj_p>>;

  class Router;

  /////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////

  class Structure : public Rec {
  protected:
    ptr<MutexDeque<Mail_p>> outbox_ = std::make_shared<MutexDeque<Mail_p>>();
    ptr<MutexDeque<Subscription_p>> subscriptions_ = std::make_shared<MutexDeque<Subscription_p>>();
    Map<ID, Inst_p> on_read_qs_ = Map<ID, Inst_p>();
    Map<ID, Inst_p> on_write_qs_ = Map<ID, Inst_p>();
    std::atomic_bool available_ = std::atomic_bool(false);

  public:
    const Pattern_p pattern;

    explicit Structure(const Pattern &pattern, const ID_p &type_id, const ID_p &value_id = nullptr,
                       const Rec_p &config = Obj::to_rec()) :
      Rec(config->rec_value()->empty()
            ? Obj::to_rec({{"pattern", vri(pattern)}})->rec_value()
            : Obj::to_rec({{"pattern", vri(pattern)}, {"config", config->clone()}})->rec_value(), OType::REC, type_id,
          value_id),
      pattern(p_p(pattern)) {
      /*if(!value_id && !pattern.equals("/sys/#")) {
        ROUTER_WRITE(furi_p(ID("/sys/router/struct").extend(pattern.retract_pattern())),this->clone(),true);
      }*/
    }

    template<typename STRUCTURE>
    static unique_ptr<STRUCTURE> create(const Pattern &pattern, const ID_p &value_id = nullptr,
                                        const Rec_p &config = Obj::to_rec()) {
      static_assert(std::is_base_of_v<Structure, STRUCTURE>, "STRUCTURE should be derived from Structure");
      unique_ptr<STRUCTURE> s = make_unique<STRUCTURE>(pattern, value_id, config);
      return s;
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
        throw fError(FURI_WRAP " !ystructure!! is closed", this->pattern->toString().c_str());
      Option<Mail_p> mail = this->outbox_->pop_front();
      while(mail.has_value()) {
        FEED_WATCHDOG();
        LOG_WRITE(TRACE, this, L("processing message {} for subscription {}\n",
                                 mail.value()->second->toString(), mail.value()->first->toString()));
        const Message_p message = mail.value()->second;
        const Subscription_p subscription = mail.value()->first;
        subscription->apply(message);
        mail = this->outbox_->pop_front();
      }
    }

    virtual void stop() {
      if(!this->available_.load())
        LOG_WRITE(WARN, this, L("!ystructure!! already stopped\n"));
      this->subscriptions_->clear();
      this->outbox_->get_base().clear();
      this->available_ = false;
    }

    /////////////////////////////////////////////////

    virtual void recv_unsubscribe(const ID &source, const fURI &target) {
      if(!this->available_.load())
        LOG_WRITE(ERROR, this, L("!yunable to unsubscribe!! {} from {}\n", source.toString(), target.toString()));
      else {
        this->subscriptions_->remove_if(
            [source, target](const Subscription_p &sub) {
              const bool removing =
                  sub->source()->equals(source) && (sub->pattern()->matches(target));
              if(removing)
                LOG_UNSUBSCRIBE(OK, &source, &target);
              return removing;
            });
      }
    }

    virtual void recv_subscription(const Subscription_p &subscription) {
      if(!this->available_.load()) {
        LOG_WRITE(ERROR, this, L("!yunable to receive!! {}\n", subscription->toString()));
        return;
      }
      LOG_WRITE(DEBUG, this, L("!yreceived!! {}\n", subscription->toString()));
      /////////////// DELETE EXISTING SUBSCRIPTION (IF EXISTS)
      this->recv_unsubscribe(*subscription->source(), *subscription->pattern());
      if(!subscription->on_recv()->is_noobj()) {
        /////////////// ADD NEW SUBSCRIPTION
        this->subscriptions_->push_back(subscription);
        LOG_WRITE(DEBUG, subscription.get(),
                  L("!m[!!{}!m][!b{}!m]=!gsubscribe!m=>{}\n", subscription->source()->toString(),
                    subscription->toString(), subscription->pattern()->toString()));
        /////////////// HANDLE RETAINS MATCHING NEW SUBSCRIPTION
        this->publish_retained(subscription);
      }
    }

    virtual void publish_retained(const Subscription_p &subscription) {
      const IdObjPairs list = this->read_raw_pairs(*subscription->pattern());
      for(const auto &[id, obj]: list) {
        if(!obj->is_noobj()) {
          if(id.matches(*subscription->pattern())) {
            FEED_WATCHDOG();
            subscription->apply(make_shared<Message>(id_p(id), obj,RETAIN));
          }
        }
      }
    }

    virtual bool has(const fURI &furi) {
      return furi.is_node() ? !this->read(furi)->is_noobj() : !this->read_raw_pairs(*furi_p(furi.extend("+"))).empty();
    }

    Obj_p read(const fURI_p &furi) {
      return this->read(*furi);
    }

    virtual Obj_p read(const fURI &furi) {
      if(!this->available_.load()) {
        LOG_WRITE(ERROR, this, L("!yunable to read!! {}\n", furi.toString()));
        return noobj();
      }
      const fURI furi_no_query = furi.no_query();
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////// READ QUERY PROCESSORS ////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      try {
        if(furi.has_query()) {
          const Objs_p results = Obj::to_objs();
          for(const auto &[key,values]: furi.query_values()) {
            if(this->on_read_qs_.count(ID(key))) {
              const Inst_p inst = this->on_read_qs_.at(key);
              const List<Str_p> vs = furi.query_values<Str_p>(key.c_str(), [](const string &v) {
                return Obj::to_str(v);
              });
              const Obj_p result = inst->apply(vri(furi_no_query), lst(vs));
              results->add_obj(result);
            }
          }
          return results;
        }
        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////// READ BRANCH PATTERN/ID ///////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        const IdObjPairs matches = this->read_raw_pairs(furi_no_query.is_branch()
                                                          ? furi_no_query.extend("+")
                                                          : furi_no_query);
        if(furi.is_branch()) {
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
            LOG_WRITE(TRACE, this, L("searching for base poly of: {}\n", furi.toString()));
            if(const auto pair = this->locate_base_poly(furi.retract()); pair.has_value()) {
              LOG_WRITE(TRACE, this, L("base poly found at {}: {}\n",
                                       pair->first.toString(),
                                       pair->second->toString())                  );
              const fURI_p furi_subpath = id_p(furi.remove_subpath(pair->first.as_branch().toString(), true));
              const Poly_p poly_read = pair->second; //->clone();
              Obj_p read_obj = poly_read->poly_get(vri(furi_subpath));
              return read_obj;
            }
            return Obj::to_noobj();
          }
          if(!furi.is_pattern())
            return matches.front().second;
          else {
            const Objs_p objs = Obj::to_objs();
            for(const auto &o: matches) {
              objs->add_obj(o.second);
            }
            return objs;
          }
        }
      } catch(const std::exception &e) {
        throw fError("unable to read from %s: %s", furi.toString().c_str(), e.what());
      }
    }

    Option<Pair<ID, Poly_p>> locate_base_poly(const fURI &furi) {
      auto old_furi = fURI(furi);
      auto pc_furi = make_unique<fURI>(old_furi); // force it to be a node. good or bad?
      Obj_p obj = Obj::to_noobj();
      while(pc_furi->path_length() > 0) {
        obj = this->read(*pc_furi);
        if(obj->is_poly())
          break;
        const fURI new_furi = pc_furi->retract().as_node();
        auto pc_new_furi = make_unique<fURI>(new_furi);
        pc_furi.swap(pc_new_furi);
      }
      return obj->is_poly()
               ? Option<Pair<ID, Poly_p>>(Pair<ID, Poly_p>(ID(*pc_furi), obj))
               : Option<Pair<ID, Poly_p>>();
    }

    virtual void write(const fURI &furi, const Obj_p &obj, const bool retain = RETAIN) {
      if(!this->available_.load()) {
        throw fError::create(this->vid_or_tid()->toString(), "!yunable to write!! %s to !b%s!!\n",
                             obj->toString().c_str(),
                             furi.toString().c_str());
      }
      const fURI furi_no_query = furi.no_query();
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////// READ QUERY PROCESSORS ////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      try {
        if(furi.has_query()) {
          //const Objs_p results = Obj::to_objs();
          for(const auto &[key,values]: furi.query_values()) {
            if(this->on_write_qs_.count(ID(key))) {
              const Inst_p inst = this->on_write_qs_.at(key);
              const List<Str_p> vs = furi.query_values<Str_p>(key.c_str(), [](const string &v) {
                return Obj::to_str(v);
              });
              const Obj_p result = inst->apply(vri(furi_no_query), lst(vs));
              //results->add_obj(result);
            }
          }
        }
        if(furi.has_query()) {
          ////////////////////////////////////////////////////////////////////////////////////////////////////////
          /// SUBSCRIBE ?sub={code|subscription|noobj} // noobj for unsubscribe
          ////////////////////////////////////////////////////////////////////////////////////////////////////////
          if(retain && furi.query_value("sub").has_value()) {
            if(obj->is_noobj()) {
              // unsubscribe
              this->recv_unsubscribe(
                  *(Process::current_process() ? Process::current_process()->vid : SCHEDULER_ID), furi.no_query());
            } else if(obj->is_code()) {
              // bcode for on_recv
              this->recv_subscription(Subscription::create(
                  Process::current_process() ? Process::current_process()->vid : SCHEDULER_ID, p_p(furi.no_query()),
                  obj));
            } else if(obj->is_rec() && Compiler(false, false).type_check(obj.get(), ID("/fos/q/sub"))) {
              // complete sub[=>] record
              this->recv_subscription(make_shared<Subscription>(obj));
            }
            return;
          }
        }

        //// WRITES
        if(const fURI new_furi = !furi.has_query() ? furi : furi.no_query(); new_furi.is_branch()) {
          // BRANCH (POLYS)
          if(obj->is_noobj()) {
            const IdObjPairs ids = this->read_raw_pairs(new_furi.ends_with("#") || new_furi.ends_with("#/")
                                                          ? new_furi
                                                          : new_furi.append("+"));
            // noobj
            for(const auto &[key, value]: ids) {
              this->write_raw_pairs(key, obj, retain);
            }
          } else if(obj->is_rec()) {
            // rec
            const auto remaining = Obj::to_rec();
            for(const auto &[key, value]: *obj->rec_value()) {
              if(key->is_uri()) {
                // uri key
                this->write_raw_pairs(new_furi.extend(key->uri_value()), value, retain);
              } else // non-uri key
                remaining->rec_value()->insert({key, value});
            }
            if(!remaining->rec_value()->empty()) {
              // non-uri keyed pairs written to /0
              this->write_raw_pairs(new_furi.extend("0"), remaining->clone(), retain);
            }
          } else if(obj->is_lst()) {
            // lst /0,/1,/2 indexing
            const List_p<Obj_p> list = obj->lst_value();
            for(size_t i = 0; i < list->size(); i++) {
              this->write_raw_pairs(new_furi.extend(to_string(i)), list->at(i), retain);
            }
          } else {
            // BRANCH (MONOS)
            // monos written to /0
            this->write_raw_pairs(new_furi.extend("0"), obj, retain);
          }
        } else {
          // NODE PATTERN
          if(new_furi.is_pattern()) {
            const IdObjPairs matches = this->read_raw_pairs(new_furi);
            for(const auto &[key, value]: matches) {
              this->write(key, obj, retain);
            }
          }
          //////////////////////////////////////////////////////////////////////////////////////////////////////////
          /////////////////////////////////////// WRITE NODE ID ////////////////////////////////////////////////////
          //////////////////////////////////////////////////////////////////////////////////////////////////////////
          else {
            LOG_WRITE(TRACE, this,L("searching for base poly of: {}\n", new_furi.toString()));
            if(const auto pair = this->locate_base_poly(new_furi.retract()); pair.has_value()) {
              LOG_WRITE(TRACE, this,L("base poly found at {}: {}\n",
                                      pair->first.toString(), pair->second->toString()));
              const ID_p id_insert =
                  id_p(new_furi.remove_subpath(pair->first.as_branch().toString(), true).to_node());
              const Poly_p poly_insert = pair->second; //->clone();
              poly_insert->poly_set(vri(id_insert), obj);
              distribute_to_subscribers(Message::create(id_p(new_furi), obj, retain));
              LOG_WRITE(TRACE, this, L("base poly reinserted into structure at {}: {}\n",
                                       pair->first.toString(), poly_insert->toString()));
              this->write(pair->first, poly_insert, retain); // NOTE: using write() so poly recursion happens
            } else {
              this->write_raw_pairs(new_furi, obj, retain);
            }
          }
        }
      } catch(const std::exception &e) {
        throw fError("unable to write %s to %s: %s", obj->toString().c_str(), furi.toString().c_str(), e.what());
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
    virtual void write_raw_pairs(const ID &id, const Obj_p &obj, bool retain) = 0;

    virtual IdObjPairs read_raw_pairs(const fURI &match) = 0;

  protected:
    static Obj_p strip_value_id(const Obj_p &obj) {
      return nullptr == obj->vid ? obj : Obj::create(obj->value_, obj->otype, obj->tid, nullptr);
    }

    void check_availability(const string &function) const {
      if(!this->available())
        throw fError("structure " FURI_WRAP " not available for %s", function.c_str());
    }

    virtual void distribute_to_subscribers(const Message_p &message) {
      this->subscriptions_->forEach([this,message](const Subscription_p &subscription) {
        if(message->target()->matches(*subscription->pattern()))
          this->outbox_->push_back(mail_p(subscription, message));
      });
    }

    bool has_equal_subscription_pattern(const fURI &topic, const ID &source = "") const {
      return this->subscriptions_->exists([source,topic](const Subscription_p &sub) {
        if(source.empty() && !source.equals(*sub->source()))
          return false;
        if(topic.equals(*sub->pattern()))
          return true;
        return false;
      });
    }

    List_p<Subscription_p> get_matching_subscriptions(const fURI &topic, const ID &source = "") const {
      const List_p<Subscription_p> matches = make_shared<List<Subscription_p>>();
      this->subscriptions_->forEach([topic,source,matches](const Subscription_p &subscription) {
        if(!(source.empty() && !source.equals(*subscription->source())) && topic.bimatches(*subscription->pattern()))
          matches->push_back(subscription);
      });
      return matches;
    }

    Objs_p get_subscription_objs(const fURI &pattern = "#") const {
      const Objs_p objs = Obj::to_objs();
      this->subscriptions_->forEach([pattern,objs](const Subscription_p &subscription) {
        if(subscription->pattern()->bimatches(pattern)) {
          objs->add_obj(subscription);
        }
      });
      return objs;
    }
  };

  using Structure_p = ptr<Structure>;
} // namespace fhatos

#endif
