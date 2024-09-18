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
      FOS_TRY_META
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      return mutex_data_.read<Objs_p>([this, furi]() {
        // FURI BRANCH
        if (furi->is_branch()) {
          // x/+/ const fURI new_furi = furi->is_branch() && furi->is_pattern() ? furi->extend("+") : *furi;
          if (furi->is_pattern()) {
            Objs_p objs = Obj::to_objs();
            for (const auto &[f, o]: *this->data_) {
              if (f->matches(furi->extend("+"))) {
                objs->add_obj(uri(f));
              }
            }
            return objs;
          }
          Rec_p rec = Obj::to_rec(share(Obj::RecMap<>())); // x/y/
          const Pattern match = furi->extend("+/");
          for (const auto &[f, o]: *this->data_) {
            if (f->matches(match))
              rec->rec_set(uri(f), o);
          }
          return rec->rec_value()->empty() ? noobj() : rec;
        }
        // FURI NODE
        if (furi->is_pattern()) {
          // x/+
          Objs_p objs = Obj::to_objs();
          for (const auto &[f, o]: *this->data_) {
            if (f->matches(*furi)) {
              objs->add_obj(o);
            }
          }
          return objs;
        }
        return data_->count(id_p(*furi)) ? data_->at(id_p(*furi)) : noobj(); //x/y
      });
    }
  };
} // namespace fhatos

#endif
