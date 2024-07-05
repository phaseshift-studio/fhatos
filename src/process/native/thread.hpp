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

#include <chrono>
#include <fhatos.hpp>
#include <thread>
//
#include FOS_PROCESS(process.hpp)

namespace fhatos {
  class Thread : public Process {
  public:
    std::thread *xthread;

    explicit Thread(const ID &id) : Process(id, THREAD), xthread(nullptr) {}

    ~Thread() override { delete this->xthread; }

    void setup() override { Process::setup(); }

    void stop() override {
      Process::stop();
      if (this->xthread && this->xthread->joinable() && (this->xthread->get_id() != std::this_thread::get_id())) {
        try {
          this->xthread->join();
        } catch (const std::runtime_error &e) {
          LOG_TASK(ERROR, this, "%s [process thread id: %i][current thread id: %i]\n", e.what(),
                   this->xthread->get_id(), std::this_thread::get_id());
        }
      }
    }

    void loop() override { Process::loop(); }

    void delay(const uint64_t milliseconds) override {
      std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    void yield() override { std::this_thread::yield(); }
  };
} // namespace fhatos

#endif
