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

#ifndef fhatos_external_hpp
#define fhatos_external_hpp

#include <fhatos.hpp>
#include <furi.hpp>
#include <language/obj.hpp>
#include <structure/structure.hpp>
#include <util/options.hpp>
#include <util/pubsub_helper.hpp>

namespace fhatos {
  class External : public Structure {
  protected:
    //<query, function<query, <id,result>>
    Map<fURI_p, Function<fURI_p, Map<ID_p, Obj_p>>, furi_p_less> read_functions_;
    Map<fURI_p, BiFunction<fURI_p, Obj_p, Map<ID_p, Obj_p>>, furi_p_less> write_functions_;
    Map<ID_p, Obj_p, furi_p_less> history{};

    explicit External(const Pattern &pattern,
                      const Map<fURI_p, Function<fURI_p, Map<ID_p, Obj_p>>, furi_p_less> &read_map = {},
                      const Map<fURI_p, BiFunction<fURI_p, Obj_p, Map<ID_p, Obj_p>>, furi_p_less> &write_map = {}) :
        Structure(pattern, SType::EPHEMERAL),
        read_functions_(read_map), write_functions_(write_map) {}

    virtual void write(const ID_p &id, const Obj_p &obj, bool retain) override {
      Map<ID_p, Obj_p> map;
      for (const auto &[furi, func]: this->write_functions_) {
        if (id->matches(*furi) || furi->matches(*id)) {
          for (const auto &[key, value]: func(id, obj)) {
            map.insert({key, value});
          }
          Options::singleton()->scheduler<Scheduler>()->feed_local_watchdog();
        }
      }
      if (!retain || map.empty())
        return;
      for (const auto &sub: *this->subscriptions_) {
        this->publish_retained_map(map, sub);
      }
    }

    virtual Obj_p read(const fURI_p &target) override {
      const fURI_p temp = target->is_pattern() ? target : (target->is_branch() ? share(target->extend("+")) : target);
      Obj::RecMap_p<Uri_p, Obj_p> map = share(Obj::RecMap<Uri_p, Obj_p>());
      for (const auto &[furi, func]: this->read_functions_) {
        if (temp->matches(*furi) || furi->matches(*temp)) {
          for (const auto &[key, value]: func(temp)) {
            map->insert({uri(key), value});
          }
          Options::singleton()->scheduler<Scheduler>()->feed_local_watchdog();
        }
      }
      if (map->empty())
        return noobj();
      if (!target->is_pattern()) {
        if (target->is_node())
          return map->begin().value();
        else
          return Obj::to_rec(map);
      }
      Objs_p objs = Obj::to_objs();
      for (const auto &[k, v]: *map) {
        objs->add_obj(v->clone());
      }
      return objs;
    }

    void loop() override {
      for (const auto &sub: *this->subscriptions_) {
        this->publish_retained(sub);
      }
    }

    void publish_retained_map(const Map<ID_p, Obj_p> &map, const Subscription_p &subscription) {
      for (const auto &[id, obj]: map) {
        if (id->matches(subscription->pattern)) {
          if (history.count(id) && !history.at(id)->equals(*obj)) {
            this->outbox_->push_back(mail_p(subscription, message_p(*id, obj, RETAIN_MESSAGE)));
            history.erase(id);
            if (!obj->is_noobj()) {
              history.insert({id, obj});
            }
          }
          Options::singleton()->scheduler<Scheduler>()->feed_local_watchdog();
        }
      }
    }


    virtual void publish_retained(const Subscription_p &subscription) override {
      for (const auto &[furi, func]: read_functions_) {
        if (furi->matches(subscription->pattern)) {
          Map<ID_p, Obj_p> map;
          for (const auto &[furi, func]: this->read_functions_) {
            if (furi->matches(subscription->pattern)) {
              for (const auto &[i, o]: func(furi_p(subscription->pattern))) {
                if (history.count(i) && !history.at(i)->equals(*o)) {
                  this->outbox_->push_back(mail_p(subscription, message_p(*i, o, RETAIN_MESSAGE)));
                  history.erase(i);
                  if (!o->is_noobj()) {
                    history.insert({i, o});
                  }
                }
              }
            }
          }
        }
      }
    }

    /*
    virtual void remove(const ID_p& target) override {
     if(this->read_functions_.count(Pattern(*target)))
        this->read_functions_.erase(Pattern(*target));
    }*/
  };
} // namespace fhatos

#endif
