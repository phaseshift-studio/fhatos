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

#ifndef fhatos_f_memory_hpp
#define fhatos_f_memory_hpp


#include <fhatos.hpp>
#include <process/actor/actor.hpp>
#include <structure/furi.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(fiber.hpp)
#include FOS_PROCESS(scheduler.hpp)

namespace fhatos {
  template<typename PROCESS = Fiber, typename ROUTER = Router >
  class fMemory : public Actor<PROCESS, ROUTER> {
  public:
    static fMemory *singleton() {
      static fMemory memory = fMemory();
      return &memory;
    }

    void setup() override {
      Actor<PROCESS, ROUTER>::setup();
      this->onQuery(this->id().query("?"), [this](const SourceID, const TargetID &target) {
        char temp[512];
        sprintf(temp,
                FOS_TAB "!b\\_!!!r%s!!\n" FOS_TAB "" FOS_TAB "" FOS_TAB
                "[!gsketch:!b" FOS_BYTES_MB_STR "!!][!gheap!!:!b" FOS_BYTES_MB_STR "!!][!gpram!!:!b" FOS_BYTES_MB_STR
                "!!][!gflash!!:!b" FOS_BYTES_MB_STR "!!]\n",
                target.toString().c_str(),
                FOS_BYTES_MB(ESP.getFreeSketchSpace()),
                FOS_BYTES_MB(ESP.getFreeHeap()),
                FOS_BYTES_MB(ESP.getFreePsram()),
                FOS_BYTES_MB(ESP.getFlashChipSize()));
        this->publish(target, string(temp),RETAIN_MESSAGE);
      });
    }

  protected:
    fMemory(const ID &id = Router::mintID("kernel", "memory")) : Actor<PROCESS, ROUTER>(id) {
    }
  };
};

#endif
