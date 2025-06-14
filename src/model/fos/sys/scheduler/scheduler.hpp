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
#ifndef fhatos_scheduler_hpp
#define fhatos_scheduler_hpp

#include "../../../../fhatos.hpp"
//
#include "../../../../furi.hpp"
#include "../router/router.hpp"
#include "thread/thread.hpp"

namespace fhatos {

  class Scheduler final : public Rec, public Mailbox {

  protected:
    Mutex mutex = Mutex();
  public:
    MutexDeque<Runnable> for_scheduler;

  public:
    explicit Scheduler(const ID &id);

    static ptr<Scheduler> &singleton(const ID &id = ID("/sys/scheduler"));

    void stop();

    void loop() override;

    void spawn_thread(const Obj_p &thread_obj);

    void bundle_fiber(const Obj_p &fiber_obj);

    void handle_bundle();

    static void *import();
  };
} // namespace fhatos
#endif
