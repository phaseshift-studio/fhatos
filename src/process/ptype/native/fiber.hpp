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
#ifndef fhatos_fiber_hpp
#define fhatos_fiber_hpp

#include <chrono>
#include <thread>
#include "process/process.hpp"

namespace fhatos {
  class Fiber : public Process {
  public:
    std::thread *xthread;
    atomic_int *FIBER_COUNT;

    explicit Fiber(const Rec_p &setup_loop_stop) :
      Process(setup_loop_stop), xthread(nullptr) {
    }

    void stop() override {
      Process::stop();
      if (0 == FIBER_COUNT->load() && this->xthread && this->xthread->joinable()) {
        try {
          if (this->xthread->get_id() != std::this_thread::get_id() && std::this_thread::get_id() == *scheduler_thread)
            this->xthread->join();
          else
            this->xthread->detach();
          LOG_PROCESS(INFO, this, "!bfiber bundle!! !ythread!! destroyed\n");
        } catch (const std::runtime_error &e) {
          //LOG_PROCESS(ERROR, this, "%s [process thread id: %i][current thread id: %i]\n", e.what(),
          //            this->xthread->get_id(), std::this_thread::get_id());
        }
      }
    }

    void delay(const uint64_t milliseconds) override {
      // delay to next fiber
      std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    void yield() override {
      // do nothing
    }
  };
} // namespace fhatos

#endif