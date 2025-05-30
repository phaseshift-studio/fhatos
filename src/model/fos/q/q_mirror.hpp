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
#ifndef fhatos_q_mirror_hpp
#define fhatos_q_mirror_hpp

#include "../../../fhatos.hpp"
#include "../../../structure/q_proc.hpp"

namespace fhatos {
  class QMirror final : public QProc {
  public:
    /*
    /sys/# -> /shared/comp
    */
    std::multimap<Pattern, fURI> mapping = std::multimap<Pattern, fURI>();

  public:
    explicit QMirror(const ID_p &value_id = nullptr) : QProc(id_p("/fos/q/mirror"), value_id) {
      this->Obj::rec_set("pattern", vri("mirror"));
    }

    void loop() const override {
      // do nothing
    }

    static ptr<QMirror> create(const ID &value_id) {
      // TYPE_SAVER("/fos/q/q_sub", Obj::to_rec());
      return make_shared<QMirror>(value_id.empty() ? nullptr : id_p(value_id));
    }

    void write(const QProc::POSITION pos, const fURI &furi, const Obj_p &obj, const bool retain) override {
      // const fURI furi_no_query = furi.no_query();
      if(POSITION::Q_LESS == pos) {
        // mirror writes
        for(const auto &maps: this->mapping) {
          if(furi.matches(maps.first)) {
            //  const fURI new_furi = furi.pretract(maps.first.retract_pattern()).prepend(maps.second);
            // ROUTER_WRITE(new_furi, obj, retain);
          }
        }
      }
    }

    /*Obj_p read(const QProc::POSITION pos, const fURI &furi, const Obj_p &post_read) const override {
      /// pre-read
      const fURI furi_no_query = furi.no_query();
      const Objs_p subs = Obj::to_objs();
      for(const Subscription_p &sub: *this->subscriptions_) {
        if(furi_no_query.matches(*sub->pattern())) {
          subs->add_obj(sub);
        }
      }
      LOG_WRITE(TRACE, this, L("!ypre-read!! !b{}!! -> {}\n", furi_no_query.toString(), subs->toString()));
      return subs;
    }*/

    [[nodiscard]] ON_RESULT is_pre_read() const override { return ON_RESULT::ONLY_Q; }

    [[nodiscard]] ON_RESULT is_pre_write() const override { return ON_RESULT::ONLY_Q; }

    [[nodiscard]] ON_RESULT is_q_less_write() const override { return ON_RESULT::IGNORE_Q; }

    [[nodiscard]] Obj_p read(POSITION position, const fURI &furi, const Obj_p &post_read) const override {
      return post_read;
      // do nothing for now
    }
  };
} // namespace fhatos
#endif
