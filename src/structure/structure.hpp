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
#include "pubsub.hpp"
#include "../util/mutex_deque.hpp"
#include "q_proc.hpp"
#include "qtype/q_sub.hpp"


#define FOS_TRY_META                                                                                                   \
  const Option<Obj_p> meta = this->try_meta(furi);                                                                     \
  if (meta.has_value())                                                                                                \
    return meta.value();


namespace fhatos {
  using IdObjPairs = List<Pair<ID, Obj_p>>;

  class Router;

  /////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////
  class Structure;
  using Structure_p = ptr<Structure>;

  class Structure : public Rec {
  protected:
   // ptr<MutexDeque<Mail_p>> outbox_ = std::make_shared<MutexDeque<Mail_p>>();
   // ptr<MutexDeque<Subscription_p>> subscriptions_ = std::make_shared<MutexDeque<Subscription_p>>();
    Rec_p q_procs_;
    std::atomic_bool available_ = std::atomic_bool(false);

  public:
    const Pattern_p pattern;

    explicit Structure(const Pattern &pattern,
                       const ID_p &type_id,
                       const ID_p &value_id = nullptr,
                       const Rec_p &config = Obj::to_rec()) :
      Rec(config->rec_value()->empty()
            ? Obj::to_rec({
                {"pattern", vri(pattern)},
                {"q_proc", rec({{"sub", QSub::create()}})},
            })->
            rec_value()
            : Obj::to_rec({
                {"pattern", vri(pattern)},
                {"q_proc", rec({{"sub", QSub::create()}})},
                {"config", config->clone()}})->rec_value(),
          OType::REC, type_id,
          value_id),
      pattern(p_p(pattern)) {
      this->q_procs_ = this->Obj::rec_get("q_proc");
    }

    void save() const override {
      this->rec_set("q_proc", this->q_procs_);
      Obj::save();
    }

    template<typename STRUCTURE>
    static ptr<STRUCTURE> create(const Pattern &pattern, const ID_p &value_id = nullptr,
                                 const Rec_p &config = Obj::to_rec()) {
      static_assert(std::is_base_of_v<Structure, STRUCTURE>, "STRUCTURE should be derived from Structure");
      ptr<STRUCTURE> s = make_shared<STRUCTURE>(pattern, value_id, config);
      return s;
    }

    static Structure_p add_qproc(const Structure_p &structure, const ptr<QProc> &qprocA,
                                 const ptr<QProc> &qprocB = nullptr,
                                 const ptr<QProc> &qprocC = nullptr) {
      structure->q_procs_->rec_set(vri(qprocA->q_key()), qprocA);
      LOG_WRITE(INFO, structure.get(),L("!yquery processor!! !b{}!! attached\n", qprocA->vid_or_tid()->toString()));
      if(qprocB) {
        structure->q_procs_->rec_set(vri(qprocB->q_key()), qprocB);
        LOG_WRITE(INFO, structure.get(),L("!yquery processor!! !b{}!! attached\n", qprocB->vid_or_tid()->toString()));
      }
      if(qprocC) {
        structure->q_procs_->rec_set(vri(qprocB->q_key()), qprocB);
        LOG_WRITE(INFO, structure.get(),L("!yquery processor!! !b{}!! attached\n", qprocC->vid_or_tid()->toString()));
      }
      structure->save();
      return structure;
    }

    [[nodiscard]] bool available() const { return this->available_.load(); }

    virtual void setup() {
      if(this->available_.load()) {
        LOG_WRITE(WARN, this, L("!ystructure!! already available\n"));
        return;
      }
      this->available_.store(true);
    }

    virtual void loop() {
      for(const auto &[k,o]: *this->q_procs_->rec_value()) {
        ((QProc *) o.get())->loop();
      }

      if(!this->available_.load())
        throw fError(FURI_WRAP " !ystructure!! is closed", this->pattern->toString().c_str());
    /*  Option<Mail_p> mail = this->outbox_->pop_front();
      while(mail.has_value()) {
        FEED_WATCHDOG();
        LOG_WRITE(TRACE, this, L("processing message {} for subscription {}\n",
                                 mail.value()->second->toString(), mail.value()->first->toString()));
        const Message_p message = mail.value()->second;
        const Subscription_p subscription = mail.value()->first;
        subscription->apply(message);
        mail = this->outbox_->pop_front();
      }*/
    }

    virtual void stop() {
      if(!this->available_.load())
        LOG_WRITE(WARN, this, L("!ystructure!! already stopped\n"));
   //   this->subscriptions_->clear();
   //   this->outbox_->get_base().clear();
      this->available_ = false;
    }

    /////////////////////////////////////////////////

    /* virtual void recv_unsubscribe(const ID &source, const fURI &target) {
       if(!this->available_.load())
         LOG_WRITE(ERROR, this, L("!yunable to unsubscribe!! {} from {}\n", source.toString(), target.toString()));
       else {
         this->subscriptions_->remove_if(
             [this,source, target](const Subscription_p &sub) {
               const bool removing = sub->source()->equals(source) && (sub->pattern()->matches(target));
               if(removing)
                 LOG_WRITE(DEBUG, this,
                           L("!m[!b{}!m]=!gunsubscribe!m=>[!b{}!m]!!\n", source.toString(), target.toString()));
               return removing;
             });
         this->rec_set("sub", lst(LstList(this->subscriptions_->begin(), this->subscriptions_->end())));
         this->save();
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
         this->rec_get("sub")->lst_add(subscription);
         this->save();
         this->subscriptions_->push_back(subscription);
         LOG_WRITE(DEBUG, this,L("!m[!b{}!m]=!gsubscribe!m=>[!b{}!m]!!\n", subscription->source()->toString(),
                                 pattern->toString())            );
         /////////////// HANDLE RETAINS MATCHING NEW SUBSCRIPTION
         this->publish_retained(subscription);
       }
     }*/

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

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    std::pair<QProc::ON_RESULT, Obj_p> process_query_read(const QProc::POSITION pos, const fURI &furi,
                                                          const Obj_p &obj) const {
      if(furi.has_query()) {
        const Objs_p results = Obj::to_objs();
        bool found = false;
        for(const auto &[k,o]: *this->q_procs_->rec_value()) {
          QProc *q = (QProc *) o.get();
          const QProc::ON_RESULT on_result = QProc::POSITION::PRE == pos ? q->is_pre_read() : q->is_post_read();
          if(QProc::ON_RESULT::NO_Q != on_result && furi.has_query(q->q_key().toString().c_str())) {
            found = true;
            const Obj_p q_obj = q->read(pos, furi, obj);
            if(QProc::ON_RESULT::ONLY_Q == on_result)
              return {on_result, q_obj};
            if(QProc::ON_RESULT::INCLUDE_Q == on_result)
              results->add_obj(q_obj);
          }
          FEED_WATCHDOG();
        }
        if(!found)
          throw fError::create(this->vid_or_tid()->toString(), "!rno query processor!! for !y%s!! on read",
                               furi.query());
        return {QProc::ON_RESULT::INCLUDE_Q, results->none_one_all()};
      } else {
        return {QProc::ON_RESULT::NO_Q, obj};
      }
    }

    QProc::ON_RESULT process_query_write(const QProc::POSITION position, const fURI &furi, const Obj_p &obj,
                                         const bool retain) const {
      if(QProc::POSITION::Q_LESS == position) {
        for(const auto &[k,o]: *this->q_procs_->rec_value()) {
          auto q = (QProc *) o.get();
          if(QProc::ON_RESULT::NO_Q != q->is_q_less_write()) {
            q->write(position, furi, obj, retain);
          }
        }
      } else if(furi.has_query()) {
        bool found = false;
        for(const auto &[k,o]: *this->q_procs_->rec_value()) {
          auto q = (QProc *) o.get();
          QProc::ON_RESULT on_result = position == QProc::POSITION::PRE ? q->is_pre_write() : q->is_post_write();
          if(QProc::ON_RESULT::NO_Q != on_result && furi.has_query(q->q_key().toString().c_str())) {
            found = true;
            q->write(position, furi, obj, retain);
            if(QProc::ON_RESULT::ONLY_Q == on_result)
              return on_result;
          }
          FEED_WATCHDOG();
        }
        if(!found)
          throw fError::create(this->vid_or_tid()->toString(), "!rno query processor!! for !y%s!! on write",
                               furi.query());
      }
      return QProc::ON_RESULT::INCLUDE_Q;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////

    virtual Obj_p read(const ID &source, const fURI &pattern) {
      return this->read(pattern);
    }

    virtual Obj_p read(const fURI &furi) {
      if(!this->available_.load()) {
        LOG_WRITE(ERROR, this, L("!yunable to read!! {}\n", furi.toString()));
        return noobj();
      }

      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////// READ QUERY PROCESSORS ////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      try {
        Objs_p results = Obj::to_objs();
        if(furi.has_query()) {
          auto [on_result, q_obj] = this->process_query_read(QProc::POSITION::PRE, furi, nullptr);
          if(on_result == QProc::ON_RESULT::ONLY_Q)
            return q_obj;
          results->add_obj(q_obj);
        }
        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////// READ BRANCH PATTERN/ID ///////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////////////
        const fURI furi_no_query = furi.no_query();
        IdObjPairs matches = this->read_raw_pairs(furi_no_query.is_branch()
                                                    ? furi_no_query.extend("+")
                                                    : furi_no_query);
        if(furi_no_query.is_branch()) {
          const Rec_p rec = Obj::to_rec();
          // BRANCH ID AND PATTERN
          for(const auto &[key, value]: matches) {
            rec->rec_set(vri(key), value, false);
          }
          results->add_obj(rec);
        } else {
          //////////////////////////////////////////////////////////////////////////////////////////////////////////
          /////////////////////////////////////// READ NODE PATTERN/ID /////////////////////////////////////////////
          //////////////////////////////////////////////////////////////////////////////////////////////////////////
          if(matches.empty()) {
            LOG_WRITE(TRACE, this, L("searching for base poly of: {}\n", furi_no_query.toString()));
            if(const auto pair = this->locate_base_poly(furi_no_query.retract()); pair.has_value()) {
              LOG_WRITE(TRACE, this, L("base poly found at {}: {}\n",
                                       pair->first.toString(),
                                       pair->second->toString())                  );
              const fURI furi_subpath = furi_no_query.remove_subpath(pair->first.as_branch().toString(), true);
              const Poly_p poly_read = pair->second; //->clone();
              const Obj_p read_obj = poly_read->poly_get(vri(furi_subpath));
              results->add_obj(read_obj);
            } else if(matches.empty()) {
              results->add_obj(Obj::to_noobj());
            }
          } else if(!furi_no_query.is_pattern())
            results->add_obj(matches.front().second);
          else {
            const Objs_p objs = Obj::to_objs();
            for(const auto &o: matches) {
              objs->add_obj(o.second);
            }
            results->add_obj(objs);
          }
        }
        return this->process_query_read(QProc::POSITION::POST, furi, results).second->none_one_all();
      } catch(const std::exception &e) {
        throw fError("unable to read from %s\n\t %s", furi.toString().c_str(), e.what());
      }
    }

    Option<Pair<ID, Poly_p>> locate_base_poly(const fURI &furi) {
      auto old_furi = fURI(furi);
      auto pc_furi = make_unique<fURI>(old_furi); // force it to be a node. good or bad?
      Obj_p obj = Obj::to_noobj();
      while(pc_furi->path_length() > 0) {
        obj = this->read(*pc_furi);
        if(obj->is_poly() || obj->is_objs())
          break;
        const fURI new_furi = pc_furi->retract().as_node();
        auto pc_new_furi = make_unique<fURI>(new_furi);
        pc_furi.swap(pc_new_furi);
      }
      return obj->is_poly() || obj->is_objs()
               ? Option<Pair<ID, Poly_p>>(Pair<ID, Poly_p>(ID(*pc_furi), obj))
               : Option<Pair<ID, Poly_p>>();
    }

    virtual void write(const ID &source, const fURI &target, const Obj_p &obj) {
      this->write(target, obj, true);
    }

    virtual void write(const fURI &furi, const Obj_p &obj, const bool retain = RETAIN) {
      if(!this->available_.load()) {
        throw fError::create(this->vid_or_tid()->toString(), "!yunable to write!! %s to !b%s!!",
                             obj->toString().c_str(),
                             furi.toString().c_str());
      }
      if(furi.has_query()) {
        if(const QProc::ON_RESULT result = this->process_query_write(QProc::POSITION::PRE, furi, obj, retain);
          result == QProc::ON_RESULT::ONLY_Q)
          return;
      }
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////// READ QUERY PROCESSORS ////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      try {
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
              const fURI id_insert = new_furi.remove_subpath(pair->first.as_branch().toString(), true).to_node();
              const Poly_p poly_insert = pair->second; //->clone();
              poly_insert->poly_set(vri(id_insert), obj);
              //distribute_to_subscribers(Message::create(id_p(new_furi), obj, retain));
              LOG_WRITE(TRACE, this, L("base poly reinserted into structure at {}: {}\n",
                                       pair->first.toString(), poly_insert->toString()));
              this->write(pair->first, poly_insert, retain); // NOTE: using write() so poly recursion happens
            } else {
              this->write_raw_pairs(new_furi, obj, retain);
            }
          }
        }
        this->process_query_write(QProc::POSITION::POST, furi, obj, retain);
        this->process_query_write(QProc::POSITION::Q_LESS, furi, obj, retain);
      } catch(const std::exception &e) {
        throw fError("unable to write %s to %s\n\t %s", obj->toString().c_str(), furi.toString().c_str(), e.what());
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

   /* virtual void distribute_to_subscribers(const Message_p &message) {
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

    Objs_p get_subscription_objs(const fURI &pattern = "#") const {
      const Objs_p objs = Obj::to_objs();
      this->subscriptions_->forEach([pattern,objs](const Subscription_p &subscription) {
        if(subscription->pattern()->bimatches(pattern)) {
          objs->add_obj(subscription);
        }
      });
      return objs;
    }*/
  };
} // namespace fhatos

#endif
