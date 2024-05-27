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

#ifndef fhatos_f_scheduler_hpp
#define fhatos_f_scheduler_hpp


#include <fhatos.hpp>
#include <process/actor/actor.hpp>
#include <structure/furi.hpp>
#include <structure/io/net/f_wifi.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(scheduler.hpp)

namespace fhatos {
  template<typename PROCESS = Thread, typename ROUTER = FOS_DEFAULT_ROUTER >
  class fScheduler : public Actor<PROCESS, ROUTER> {
  public:
    static fScheduler *singleton() {
      static fScheduler scheduler = fScheduler();
      return &scheduler;
    }

    void setup() override {
      Actor<PROCESS, ROUTER>::setup();
      this->onQuery(this->id().query("?"), [this](const SourceID&, const TargetID& target) {
        char temp[100];
        sprintf(temp, "\\_%s", target.query("").toString().c_str());
        this->publish(target, temp,RETAIN_MESSAGE);
      });
    }

    void loop() override {

    }

    bool spawn(Process *process) {
      return Scheduler::singleton()->spawn(process);
    }

  protected:
    fScheduler(const ID &id = fWIFI::idFromIP("kernel", "scheduler")) : Actor<PROCESS, ROUTER>(id) {
    }
  };
};

#endif
