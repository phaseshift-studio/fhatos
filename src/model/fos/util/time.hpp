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
#ifndef fhatos_util_time_hpp
#define fhatos_util_time_hpp

#include "../../../fhatos.hpp"
#include "../../../furi.hpp"
#include "../sys/typer/typer.hpp"

namespace fhatos {
  static const ID_p TIME_FURI = id_p(FOS_URI "/util/time");
  class Time final : public Int {
  protected:
    std::chrono::steady_clock::time_point start_time;

  public:
    explicit Time(const Obj_p &time_obj) :
        Real(Obj(time_obj->value_, OType::REAL, TIME_FURI, time_obj->vid)),
        start_time(std::chrono::steady_clock::now()) {}

    void setup() override {
      if(!this->vid)
        throw fError("!yvid !rrequired !yfor !b%s!!", TIME_FURI->toString().c_str());
      Time *t = this->as(TIME_FURI)->get_model<Time>();
      t->reset();
    }

    void reset() { this->start_time = std::chrono::steady_clock::now(); }

    [[nodiscard]] FOS_REAL_TYPE duration() const {
      const auto endTime = std::chrono::steady_clock::now();
      const auto duration = std::chrono::duration<double>(endTime - this->start_time);
      const_cast<any *>(&this->value_)->emplace<FOS_REAL_TYPE>(duration.count());
      return duration.count();
    }

    /* [[nodiscard]] ptr<const Time> shared_with_this() const {
       return this->get_shared_from_this<Time>();
     }*/

    static void register_module() {
      MODEL_CREATOR2->insert_or_assign(*TIME_FURI, [](const Obj_p &time_obj) {
        auto t = make_shared<Time>(time_obj);
        t->reset();
        return t;
      });
      ////////////////////////// TYPE ////////////////////////////////
      REGISTERED_MODULES->insert_or_assign(
          *TIME_FURI, InstBuilder::build(Typer::singleton()->vid->add_component(*TIME_FURI))
                          ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {1, 1})
                          ->inst_f([](const Obj_p &, const InstArgs &) {
                            return Obj::to_rec({{vri(*TIME_FURI), __().isa(*REAL_FURI)},
                                                {vri(TIME_FURI->add_component("reset")),
                                                 InstBuilder::build(TIME_FURI->add_component("reset"))
                                                     ->domain_range(TIME_FURI, {1, 1}, TIME_FURI, {1, 1})
                                                     ->inst_f([](const Obj_p &obj, const InstArgs &) {
                                                       Time *t = obj->get_model<Time>();
                                                       LOG_WRITE(INFO, obj.get(),
                                                                 L("!ytime since start!!: {}!gs!!\n", t->duration()));
                                                       t->reset();
                                                       return t->shared_from_this();
                                                     })
                                                     ->create()}});
                          })
                          ->create());
    }
  };
} // namespace fhatos
#endif
