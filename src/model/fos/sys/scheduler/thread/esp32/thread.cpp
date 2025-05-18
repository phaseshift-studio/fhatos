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
#ifdef ARDUINO
#include "../thread.hpp"
#include <any>
#include "../../../../../../fhatos.hpp"
#include "../../../../../../lang/obj.hpp"

namespace fhatos {

  static void THREAD_FUNCTION(void *vptr_thread) {
    auto *fthread = static_cast<Thread *>(vptr_thread);
    if(!fthread) {
      LOG_WRITE(ERROR, Obj::to_noobj().get(), L("unable to acquire thread state"));
      return;
    }
    if(!fthread->thread_obj_) {
      LOG_WRITE(ERROR, Obj::to_noobj().get(), L("unable to acquire thread obj"));
      return;
    }
    if(!fthread->thread_function_) {
      LOG_WRITE(ERROR, fthread->thread_obj_.get(),
                L("unable to acquire thread function: {}", fthread->thread_obj_->toString()));
      return;
    }
    try {
      int stack_size =
          Memory::get_stack_size(fthread->thread_obj_, "config/stack_size",
                                 ROUTER_READ(SCHEDULER_ID->extend("config/def_stack_size"))->or_else_(129536));
      static Thread *THREAD_PTR = fthread;
      Memory::singleton()->use_custom_stack(InstBuilder::build("thread_stack")
                                                ->inst_f([](const Obj_p &, const InstArgs &) {
                                                  THREAD_PTR->thread_function_(
                                                      std::make_pair<>(THREAD_PTR, THREAD_PTR->thread_obj_));
                                                  return Obj::to_noobj();
                                                })
                                                ->create(),
                                            Obj::to_noobj(), stack_size);
      vTaskDelete(nullptr);
    } catch(const std::exception &e) {
      LOG_WRITE(ERROR, fthread->thread_obj_.get(), L("{}", e.what()));
    }
  }

  Thread::Thread(const Obj_p &thread_obj, const Consumer<std::pair<Thread *, Obj_p>> &thread_function) :
      thread_obj_(thread_obj), thread_function_(thread_function), handler_(std::make_any<TaskHandle_t *>(nullptr)) {
    int stack_size = Memory::get_stack_size(
        thread_obj, "config/stack_size", ROUTER_READ(SCHEDULER_ID->extend("config/def_stack_size"))->or_else_(64768));
    if(0 == stack_size)
      throw fError("thread stack size must be greater than 0");
    LOG_WRITE(INFO, thread_obj.get(),
              L("!g[!besp32!g] !ythread!! spawned: {} !m[!ystack size:!!{}!m]!!\n",
                thread_obj_->obj_get("loop")->toString(), stack_size));
    const BaseType_t threadResult = xTaskCreatePinnedToCore(THREAD_FUNCTION, // function that should be called
                                                            this->thread_obj_->vid->toString().c_str(),
                                                            // name of the task (for debugging)
                                                            FOS_PRE_PSRAM_THREAD_STACK_SIZE, // stack size (bytes)
                                                            static_cast<void *>(this), // parameter to pass
                                                            1, // CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, // task priority
                                                            this->get_handler<TaskHandle_t *>(), // task handle
                                                            tskNO_AFFINITY); // processor core
    if(pdPASS != threadResult)
      throw fError("unable to spawn thread: %s", this->thread_obj_->toString().c_str());
  }


  void Thread::halt() const {
    vTaskDelete(nullptr);
    delete this;
  }

  void Thread::yield() { taskYIELD(); }

  void Thread::delay(const uint64_t milliseconds) { vTaskDelay(milliseconds / portTICK_PERIOD_MS); }
} // namespace fhatos
#endif
