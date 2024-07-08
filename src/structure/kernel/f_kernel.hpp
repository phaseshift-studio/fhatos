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

#ifndef fhatos_f_kernel_hpp
#define fhatos_f_kernel_hpp


#include <fhatos.hpp>
#include <process/actor/actor.hpp>
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(scheduler.hpp)

namespace fhatos {
  template<typename PROCESS = Thread>
  class fKernel : public Actor<PROCESS> {
  public:
    static const bool bootloader(const List<Process *> &processes) {
#ifndef NATIVE
      Serial.begin(FOS_SERIAL_BAUDRATE);
#endif
      LOG(NONE, ANSI_ART);
      LOG(INFO, "!g[kernel mode] !bBootloader started!!\n");
      const fKernel<> *kernel = fKernel<>::singleton("/kernel/");
      bool success = Scheduler::singleton()->spawn((Process *) kernel);
      if (!success) {
        LOG(ERROR, "!rUnable to construct !b%s!!\n", kernel->id()->toString().c_str());
      } else {
        for (auto *process: processes) {
          success = success & Scheduler::singleton()->spawn(process);
          if (!success) {
            LOG(ERROR, "!rUnable to construct !b%s!!\n", process->id()->toString().c_str());
            break;
          }
        }
      }
      LOG(INFO, "!g[kernel mode] !bBootloader finished!!\n");
      return success;
    }

    static fKernel *singleton(const ID &id = ID("/kernel/")) {
      static fKernel kernel = fKernel(id);
      return &kernel;
    }

    void setup() override {
      PROCESS::setup();
      this->subscribe("user/#", [this](const Message_p &message) {
        const Obj_p obj = message->payload;
        const fURI userId = message->target.path(this->id()->pathLength());
        if (obj->isNoObj()) {
          LOG(DEBUG, "!yInitiating user destruction: !b%s!!\n", userId.toString().c_str());
          Router::destroy(ID(string("/home/") + userId.toString()), *this->id());
        } else {
          if (obj->o_type() != OType::REC) {
            LOG(ERROR, "Provided obj must be a /rec/user %s\n", OTypes.toChars(obj->o_type()));
          } else {
            LOG(DEBUG, "!gInitiating user construction: !b%s!!\n", userId.toString().c_str());
            Router::write(ID(string("/home/") + userId.toString()),
                          Obj::to_str(string("home location of ") + userId.toString()));
          }
        };
      });
    }

  protected:
    explicit fKernel(const ID &id = ID("/kernel/")) : Actor<PROCESS>(id) {}
  };
}; // namespace fhatos

#endif
