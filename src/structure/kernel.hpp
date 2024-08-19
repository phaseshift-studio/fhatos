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

#ifndef fhatos_kernel_hpp
#define fhatos_kernel_hpp

#include <fhatos.hpp>
#include FOS_PROCESS(scheduler.hpp)
#include <language/exts.hpp>
#include <process/actor/actor.hpp>
#include <process/process.hpp>
#include <model/terminal.hpp>

#include <language/types.hpp>
namespace fhatos {
  class Kernel {
  public:
    static ptr<Kernel> build() {
      static Kernel kernel = Kernel();
      static ptr<Kernel> kernel_p = PtrHelper::no_delete(&kernel);
      return kernel_p;
    }
    static ptr<Kernel> with_log_level(const LOG_TYPE level) {
      Options::singleton()->log_level(level);
      return Kernel::build();
    }
    static ptr<Kernel> using_printer(const ptr<Ansi<>> &ansi) {
      Options::singleton()->printer<Ansi<>>(ansi);
      return Kernel::build();
    }
    static ptr<Kernel> displaying_splash(const char *splash) {
      printer<>()->print(splash);
      return Kernel::build();
    }
    static ptr<Kernel> displaying_notes(const char *notes) {
      printer<>()->printf(FOS_TAB_4 "%s\n", notes);
      return Kernel::build();
    }
    static ptr<Kernel> using_scheduler(const ptr<Scheduler> &scheduler) {
      Options::singleton()->scheduler<Scheduler>(scheduler);
      return Kernel::build();
    }
    static ptr<Kernel> using_router(const ptr<Router> &router) {
      Options::singleton()->router<Router>(router);
      return Kernel::build();
    }
    template<typename ACTOR>
    static ptr<Kernel> boot(const ptr<ACTOR> bootable) {
      router()->attach(bootable);
      scheduler()->spawn(bootable);
      return Kernel::build();
    }
    static ptr<Kernel> load_modules(const List<ID> &modules) {
      for (const ID &id: modules) {
        // List_p<Obj_p> list = share(List<Obj_p>());
        for (const Pair<ID, Type_p> &pair: Exts::exts(id)) {
          const ID_p idp = share(pair.first);
          Types::singleton()->saveType(idp, pair.second);
          // list->push_back(Obj::to_uri(*idp));
        }
        // router<Router>()->publish(
        //    Message{.source = FOS_DEFAULT_SOURCE_ID, .target = id, .payload = Obj::to_lst(list), .retain = true});
      }
      return Kernel::build();
    }
    static ptr<Kernel> initial_terminal_owner(const ID &output) {
      Terminal::currentOut(share(output));
      return Kernel::build();
    }
    static void done(const char *barrier = "kernel_barrier") {
      Scheduler::singleton()->barrier(barrier, nullptr,
                                      FOS_TAB_3 "!mPress!! <!yenter!!> !mto access terminal!! !gI/O!!");
      printer()->printf("\n" FOS_TAB_8 "%s !mFhat!gOS!!\n\n", Ansi<>::sillyPrint("shutting down").c_str());
    }
  };
} // namespace fhatos

#endif
