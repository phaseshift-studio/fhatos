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
#ifndef fhatos_thread_hpp
#define fhatos_thread_hpp

#include "../../../fhatos.hpp"
#include "../../process.hpp"

namespace fhatos {
  class Thread : public Process {
  protected:
    int find_stack_size() {
      return ROUTER_READ(this->vid->extend("stack_size")) // check provided obj
          ->or_else(ROUTER_READ(this->vid->extend("+/stack_size"))->none_one())
          // check one depth more (e.g. config/stack_size)
          ->or_else(ROUTER_READ(SCHEDULER_ID->extend("config/def_stack_size")))
          // check default setting in scheduler
          ->or_else(jnt(FOS_ESP_THREAD_STACK_SIZE)) // use default environmental variable
          ->int_value();
    }

  public:
    TaskHandle_t* xthread;

    explicit Thread(const Obj_p &obj) : Process(obj), xthread(nullptr) {

    }

    /*void setup() override {
      Process::setup();
      const ID_p hwm_id = id_p(this->vid->extend(":hwm"));
      InstBuilder::build(hwm_id)
          ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {1, 1})
          ->inst_f([this](const Obj_p &, const InstArgs &) {
            const FOS_INT_TYPE ssize = this->find_stack_size();
            const FOS_INT_TYPE sfree = ssize - uxTaskGetStackHighWaterMark(this->handle);
            return Obj::to_rec({
              {"total", jnt(ssize)},
              {"min_free", jnt(sfree)},
              {"used", real(ssize == 0
                              ? 0.0f
                              : (100.0f * (1.0f - static_cast<float>(sfree) / static_cast<float>(ssize))))}});
          })->create(hwm_id);
    }*/

    void yield() override {
      Process::yield();
      taskYIELD();
    }

    void delay(const uint64_t milliseconds) override {
      Process::delay(milliseconds);
      vTaskDelay(milliseconds / portTICK_PERIOD_MS);
    }
  };
} // namespace fhatos

#endif
