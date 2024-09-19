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

    List<ID_p> existing_ids(const fURI &match) override {
      return this->mutex_data_.read<List<ID_p>>([this,match]() {
        List<ID_p> ids;
        for (const auto &[id, obj]: *this->data_) {
          if (id->matches(match))
            ids.push_back(id);
        }
        return ids;
      });
    }

    void write(const ID_p &target, const Obj_p &payload, const bool retain) override {
      if (retain) {
        const Map<ID_p, Obj_p> map = this->generate_write_input(target, payload);
        this->mutex_data_.write<void *>([this,map]() {
          for (const auto &[key,value]: map) {
            if (this->data_->count(key))
              this->data_->erase(key);
            if (!value->is_noobj()) {
              this->data_->insert({id_p(*key), value->clone()});
              LOG_STRUCTURE(DEBUG, this, "!g%s!y=>!g%s!! written\n", key->toString().c_str(),
                            value->toString().c_str());
            } else {
              LOG_STRUCTURE(DEBUG, this, "!g%s!y=>%s removed\n", key->toString().c_str(),
                            value->toString().c_str());
            }
          }
          return nullptr;
        });
      }
      distribute_to_subscribers(message_p(*target, payload, retain));
    }

    Obj_p read(const fURI_p &furi) override {
      return this->mutex_data_.read<Obj_p>([this,furi]() {
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
      });
    }
  };
} // namespace fhatos

#endif
