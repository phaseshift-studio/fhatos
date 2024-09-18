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
#ifndef fhatos_key_value_hpp
#define fhatos_key_value_hpp

#include "fhatos.hpp"
#include "language/obj.hpp"
#include "structure/structure.hpp"
#include <language/processor/algorithm.hpp>

namespace fhatos {
  class KeyValue : public Structure {
  protected:
    ptr<Map<ID_p, const Obj_p, furi_p_less>> data_ /*PROGMEM*/ = share(
      Map<ID_p, const Obj_p, furi_p_less>());
    MutexRW<> mutex_data_ = MutexRW<>("<key value data>");

    explicit KeyValue(const Pattern &pattern, const SType stype = SType::DATABASE) : Structure(pattern, stype) {
    };

  public:
    static ptr<KeyValue> create(const Pattern &pattern) {
      auto kv_p = ptr<KeyValue>(new KeyValue(pattern));
      return kv_p;
    }

    void stop() override {
      Structure::stop();
      data_->clear();
    }

    void publish_retained(const Subscription_p &subscription) override {
      this->mutex_data_.read<void *>([this, subscription]() {
        for (const auto &[id,obj]: *this->data_) {
          if (id->matches(subscription->pattern)) {
            if (!obj->is_noobj()) {
              subscription->on_recv->apply(Message{.target = *id, .payload = obj, .retain = RETAIN_MESSAGE}.to_rec());
            }
          }
        }
        return nullptr;
      });
    }

    void write(const ID_p &target, const Obj_p &payload, const bool retain) override {
      if (retain) {
        const Bool_p embed = this->mutex_data_.write<Bool>([this, target, &payload]() {
          // BRANCH
          if (payload->is_noobj()) {
            if (data_->count(target))
              data_->erase(target);
            LOG_STRUCTURE(DEBUG, this, "!g%s!y=>%s removed\n", target->toString().c_str(),
                          payload->toString().c_str());
          } else {
            if (target->is_branch())
              return dool(true);
            // NODE
            if (data_->count(target))
              data_->erase(target);
            data_->insert({target, payload->clone()}); // why such a deep copy needed?
            LOG_STRUCTURE(DEBUG, this, "!g%s!y=>!g%s!! written\n", target->toString().c_str(),
                          payload->toString().c_str());
          }
          return dool(false);
        });
        if (embed->bool_value())
          Algorithm::embed(payload, target, PtrHelper::no_delete(this));
      }
      distribute_to_subscribers(message_p(*target, payload, retain));
    }

    Obj_p read(const fURI_p &furi) override {
      if (furi->is_node() && !furi->is_pattern())
        return this->data_->count(id_p(*furi)) ? this->data_->at(id_p(*furi)) : noobj();
      //////////////////////////////////////////////////////////////////////////////////
      Map<ID_p, Obj_p> matches;
      const fURI temp = furi->is_branch() ? furi->extend("+") : *furi;
      for (const auto &[key,value]: *this->data_) {
        if (key->matches(temp))
          matches.insert({key, value});
      }
      return generate_read_output(furi, matches);
    }

    /* virtual Obj_p read_branch_pattern(const Pattern &branch_pattern) const override {
       Objs_p objs = Obj::to_objs();
       for (const auto &[f, o]: *this->data_) {
         if (f->matches(branch_pattern.extend("+"))) {
           objs->add_obj(uri(f));
         }
       }
       return objs;
     }

     virtual Rec_p read_branch_id(const ID &branch_id) const override {
       Rec_p rec = Obj::to_rec(share(Obj::RecMap<>())); // x/y/
       const Pattern match = branch_id.extend("+");
       for (const auto &[f, o]: *this->data_) {
         if (f->matches(match))
           rec->rec_set(uri(f), o);
       }
       return rec->rec_value()->empty() ? noobj() : rec;
     }

     virtual Objs_p read_node_pattern(const Pattern &node_pattern) const override {
       Objs_p objs = Obj::to_objs();
       for (const auto &[f, o]: *this->data_) {
         if (f->matches(node_pattern)) {
           objs->add_obj(o);
         }
       }
       return objs;
     }

     virtual Obj_p read_node_id(const ID &node_id) const override {
       return data_->count(id_p(node_id)) ? data_->at(id_p(node_id)) : noobj();
     }*/
  };
} // namespace fhatos

#endif
