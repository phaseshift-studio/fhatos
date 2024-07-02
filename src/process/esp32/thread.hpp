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

#ifndef fhatos_thread_hpp
#define fhatos_thread_hpp

#include <fhatos.hpp>
//
#include FOS_PROCESS(process.hpp)

namespace fhatos {
  class Thread : public Process {
  public:
    TaskHandle_t handle;

    explicit Thread(const ID &id) : Process(id, THREAD) {
    }

    void setup() override {
      Process::setup();
    }

    void stop() override {
      Process::stop();
    }

    void delay(const uint64_t milliseconds) override {
      vTaskDelay(milliseconds / portTICK_PERIOD_MS);
    }

    void yield() override { taskYIELD(); }
  };
} // namespace fhatos

#endif
