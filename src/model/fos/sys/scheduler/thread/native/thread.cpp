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
#ifdef NATIVE
#include "../thread.hpp"
#include <chrono>
#include <thread>
#include "../../../../../../fhatos.hpp"
#include "../../../../../../lang/obj.hpp"

namespace fhatos {

  Thread::Thread(const Obj_p &thread_obj, const Consumer<Thread *> &thread_function) :
      thread_function_(thread_function), handler_(std::make_any<std::thread *>(new std::thread(thread_function, this))),
      thread_obj_(thread_obj) {
    const int cpu = sched_getcpu();
    LOG_WRITE(INFO, thread_obj.get(),
              L("!g[!bnative!g] !ythread!! spawned: {} !m[!ycpu:!!{}!m]!!\n",
                this->thread_obj_->obj_get("loop")->toString(), cpu));
  }

  void Thread::halt() const {
    if(const auto xthread = this->get_handler<std::thread *>(); xthread->joinable()) {
      try {
        if(this->get_handler<std::thread *>()->get_id() != std::this_thread::get_id()
           /*&& std::this_thread::get_id() == *scheduler_thread*/)
          xthread->join();
        else
          xthread->detach();
        delete xthread;
      } catch(const std::runtime_error &e) {
        fError::create(this->thread_obj_->toString(), "unable to halt thread: %s", e.what());
      }
    }
    delete this;
  }

  void Thread::delay(const uint64_t milliseconds) { std::this_thread::sleep_for(chrono::milliseconds(milliseconds)); }

  void Thread::yield() { std::this_thread::yield(); }
} // namespace fhatos
#endif
