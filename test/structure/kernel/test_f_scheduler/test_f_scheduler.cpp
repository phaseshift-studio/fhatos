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

#ifndef fhatos_test_f_scheduler_hpp
#define fhatos_test_f_scheduler_hpp

#include <test_fhatos.hpp>
//
#include <structure/furi.hpp>
#include FOS_PROCESS(scheduler.hpp)
#include <structure/kernel/f_scheduler.hpp>
#include <language/fluent.hpp>

namespace fhatos {

  void test_spawn() {
    Scheduler::singleton()->spawn(fScheduler<>::singleton());
    LocalRouter<>::singleton()->publish(
        Message{
            .source = ID("test"),
            .target = fScheduler<>::singleton()->id().query("?spawn"),
            .payload = BinaryObj<>::fromObj(new Rec({
                {new Str("id"), new Uri("testing")},
                {new Str("setup"), __(12).bcode.get()},
                {new Str("loop"), __(12).bcode.get()},

            }))
        });
    Scheduler::singleton()->destroy(fScheduler<>::singleton()->id());
  }

  FOS_RUN_TESTS( //
      LocalRouter<>::singleton(); //
      Scheduler::singleton(); //
      // FOS_RUN_TEST(test_spawn); //
      )
  ;
} // namespace fhatos

SETUP_AND_LOOP()

#endif
