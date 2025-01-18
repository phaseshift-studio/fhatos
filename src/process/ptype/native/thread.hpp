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
#include <thread>
#include "../../process.hpp"

namespace fhatos {
  class Thread : public Process {
  public:
    std::thread *xthread;

    explicit Thread(const ID_p &value_id, const Rec_p &setup_loop_stop) :
    Process(value_id, setup_loop_stop->rec_merge(rmap({
                                                                                 {"delay", InstBuilder::build(
                                                                                     value_id->append("/:delay"))
                                                                                   ->type_args(x(0, Obj::to_bcode()))
                                                                                   ->domain_range(
                                                                                     INT_FURI, {0, 1}, INT_FURI, {0, 1})
                                                                                   ->inst_f([this](const Obj_p &lhs, const InstArgs &args) {
                                                                                       //  ((Process*)lhs.get())->delay(args.at(0)->int_value());
                                                                                       Process::current_process()->
                                                                                           sleep_ = args->arg(0)->
                                                                                           int_value();
                                                                                       return lhs;
                                                                                     })
                                                                                   ->create()}}))),
                                                                          xthread(nullptr) {
      /*  ObjHelper::InstTypeBuilder::build(SCHEDULER_ID->extend("lib/thread/inst/delay"))
            ->type_args(x(0, Obj::to_bcode()))
            ->inst_f([](const Obj_p &lhs, const InstArgs &args) {
              ((Thread*)lhs.get())->delay(args.at(0)->int_value());
              return lhs;
            })->create(id_p(SCHEDULER_ID->extend("lib/thread/inst/delay")));*/
      //this->tid_ = PROCESS_FURI;
    }

    ~Thread() override { delete this->xthread; }

    void setup() override { Process::setup(); }

    void stop() override {
      Process::stop();
      if(this->xthread && this->xthread->joinable()) {
        try {
          if(this->xthread->get_id() != std::this_thread::get_id() && std::this_thread::get_id() == *
             scheduler_thread)
            this->xthread->join();
          else
            this->xthread->detach();
        } catch(const std::runtime_error &e) {
          //  LOG_PROCESS(ERROR, this, "%s [process thread id: %s][current thread id: %s]\n", e.what(),
          //            this->xthread->get_id(), std::this_thread::get_id());
        }
      }
    }

    void delay(const uint64_t milliseconds) override {
      std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    void yield() override {
      std::this_thread::yield();
    }
  };
} // namespace fhatos

#endif
