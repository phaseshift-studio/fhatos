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
#include <language/fluent.hpp>
#include <structure/kernel/f_kernel.hpp>
#include <structure/kernel/f_scheduler.hpp>

namespace fhatos {

  template<typename ROUTER = LocalRouter>
  void test_spawn() {
    try {
      fKernel<>::bootloader({
          // fScheduler<Thread, ROUTER>::singleton(),
      });
      Scheduler<>::singleton()->publish(
          Scheduler<ROUTER>::singleton()->id().query("?spawn"),
          BinaryObj<>::fromObj(
              (new Rec({
                   {new Str("id"), new Uri("test_spawn")},
                   {new Str("setup"), new Bytecode(__(0).ref("loop").bcode->value())},
                   {new Str("loop"),
                    new Bytecode(__(0)
                                     .dref("loop")
                                     .plus(1)
                                     .bswitch(
                                         {{_.is(_.lt(0)), _.plus(0)},
                                          {_.is(_.gt(10)), _.mult(-1).ref("loop").publish(
                                                               Scheduler<ROUTER>::singleton()->id().query("?destroy"),
                                                               Uri("test_spawn"))},
                                          {_, _.ref("loop")}})
                                     .bcode->value())},

               }))
                  ->as<Rec>("thread")));
      Scheduler<ROUTER>::singleton()->barrier("done_barrier",
                                              []() { return Scheduler<ROUTER>::singleton()->count() == 0; });

      // TEST_ASSERT_EQUAL_INT(-11,
      //                  ROUTER::singleton()->template read<Int>(fScheduler<>::singleton()->id(), "loop")->value());
    } catch (fError &error) {
      LOG_EXCEPTION(error);
    }
  }

  FOS_RUN_TESTS( //
      LocalRouter::singleton(); //
      Scheduler<>::singleton(); //
      FOS_RUN_TEST(test_spawn<LocalRouter>); //
  );
} // namespace fhatos

SETUP_AND_LOOP()

#endif
