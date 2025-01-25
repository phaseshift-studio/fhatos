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
    int hwm = -1;

  public:
    TaskHandle_t handle;

    explicit Thread(const Obj_p &obj) : Process(obj) {
      //this->hwm = this->rec_get("stack_size")->is_noobj() ? -1 : 0;
    }

    void loop() override {
      Process::loop();
      /*if(hwm != -1) {
        if(hwm++ > 100) {
          int stack_size = this->rec_get("stack_size")->int_value();
          int mark = uxTaskGetStackHighWaterMark(this->handle);
      this->rec_set("hwm", jnt(mark));
          this->save();
          hwm = 0;
        }
      }*/
    }

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
