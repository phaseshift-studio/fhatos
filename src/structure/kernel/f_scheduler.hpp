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
#include FOS_PROCESS(thread.hpp)
#include FOS_PROCESS(scheduler.hpp)
#include <structure/f_bcode.hpp>

namespace fhatos {
  template<typename PROCESS = Fiber, typename ROUTER = FOS_DEFAULT_ROUTER>
  class fScheduler final : public Actor<PROCESS, ROUTER> {
  public:
    static fScheduler *singleton() {
      static auto scheduler = fScheduler();
      return &scheduler;
    }

    void setup() override {
      Actor<PROCESS, ROUTER>::setup();
      /*this->onQuery(this->id().query("?"), [this](const SourceID &, const TargetID &target) {
        char temp[100];
        sprintf(temp, "\\_%s", target.query("").toString().c_str());
        this->publish(target, temp, RETAIN_MESSAGE);
      });*/
      this->subscribe(this->id().query("?spawn"), [this](const Message &message) {
        const Rec rec = message.payload->toRec();
        const auto b = new fBcode(rec.get<Uri>(new Str("id"))->value(), new Rec(*rec.value()));
       //this->spawn(b);
      });
      this->subscribe(this->id().query("?destroy"), [this](const Message &message) {
        const Uri uri = message.payload->toUri();
        LOG_TASK(DEBUG, this, "received ?destroy=%s from %s\n", uri.toString().c_str(),
                 message.source.toString().c_str());
       // this->destroy(uri.value());
      });
    }

    void stop() override { Actor<PROCESS, ROUTER>::stop(); }

    void loop() override { Actor<PROCESS, ROUTER>::loop(); }

  protected:
    explicit fScheduler(const ID &id = FOS_DEFAULT_ROUTER::mintID("kernel", "scheduler")) :
        Actor<PROCESS, ROUTER>(id) {}
  };
}; // namespace fhatos

#endif
