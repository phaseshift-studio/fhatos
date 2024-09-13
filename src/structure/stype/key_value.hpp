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

namespace fhatos {
  class KeyValue : public Structure {
  protected:
    ptr<Map<ID_p, Pair<const Obj_p, const ID_p>, furi_p_less>> DATA = share(
      Map<ID_p, Pair<const Obj_p, const ID_p>, furi_p_less>());
    MutexRW<> MUTEX_DATA = MutexRW<>("<key value data>");

    explicit KeyValue(const Pattern &pattern, const SType stype = SType::DATABASE) : Structure(pattern, stype) {
    };

  public:
    static ptr<KeyValue> create(const Pattern &pattern) {
      auto kv_p = ptr<KeyValue>(new KeyValue(pattern));
      return kv_p;
    }

    void stop() override {
      Structure::stop();
      DATA->clear();
    }

    void publish_retained(const Subscription_p &subscription) override {
      MUTEX_DATA.read<void *>([this, subscription]() {
        for (const auto &[id,obj]: *this->DATA) {
          if (id->matches(subscription->pattern)) {
            if (!obj.first->is_noobj()) {
              subscription->onRecv->apply(Message{
                .source = *obj.second, .target = *id, .payload = obj.first, .retain = RETAIN_MESSAGE
              }.to_rec());
            }
          }
        }
        return nullptr;
      });
    }

    void write(const ID_p &target, const Obj_p &payload, const ID_p &source, const bool retain) override {
      if (retain) {
        const Bool_p embed = MUTEX_DATA.write<Bool>([this, target, &payload, source]() {
          // BRANCH
          if (target->is_branch()) {
            return dool(true);
          } else {
            // NODE
            if (DATA->count(target))
              DATA->erase(target);
            if (!payload->is_noobj()) {
              DATA->insert({target, std::make_pair(payload->clone(), source)}); // why such a deep copy needed?
              LOG_STRUCTURE(TRACE, this, "!g%s!y=>!g%s!! written\n", target->toString().c_str(),
                            payload->toString().c_str());
            }
          }
          return dool(false);
        });
        if (embed->bool_value())
          Algorithm::embed(payload, target, source);
      }
      const Message_p message = share(Message{
        .source = *source,
        .target = *target,
        .payload = payload,
        .retain = retain
      });
      distribute_to_subscribers(message);
    }

    Obj_p read(const fURI_p &furi, [[maybe_unused]] const ID_p &source) override {
      FOS_TRY_META
      /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
      return MUTEX_DATA.read<Objs_p>([this, source, furi]() {
        // FURI BRANCH
        if (furi->is_branch()) {
          // x/+/
          if (furi->is_pattern()) {
            Objs_p objs = Obj::to_objs();
            for (const auto &[f, o]: *this->DATA) {
              if (f->matches(*furi)) {
                objs->add_obj(uri(f));
              }
            }
            return objs;
          }
          Rec_p rec = Obj::to_rec(share(Obj::RecMap<>())); // x/y/
          const Pattern match = furi->extend("+/");
          for (const auto &[f, o]: *this->DATA) {
            if (f->matches(match))
              rec->rec_set(uri(f), o.first);
          }
          return rec->rec_value()->empty() ? noobj() : rec;
        }
        // FURI NODE
        if (furi->is_pattern()) {
          // x/+
          Objs_p objs = Obj::to_objs();
          for (const auto &[f, o]: *this->DATA) {
            if (f->matches(*furi)) {
              objs->add_obj(o.first);
            }
          }
          return objs;
        }
        return DATA->count(id_p(*furi)) ? DATA->at(id_p(*furi)).first : noobj(); //x/y
      });
    }
  };
} // namespace fhatos

#endif
