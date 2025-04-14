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
#include "../../../../../../fhatos.hpp"
#include "../../../../../../lang/obj.hpp"
#include "../thread.hpp"
#include <any>

namespace fhatos {

  /* int find_stack_size() {
     return ROUTER_READ(this->thread_obj->vid->extend("stack_size")) // check provided obj
         ->or_else(ROUTER_READ(this->thread_obj->vid->extend("+/stack_size"))->none_one())
         // check one depth more (e.g. config/stack_size)
         ->or_else(ROUTER_READ(SCHEDULER_ID->extend("config/def_stack_size")))
         // check default setting in scheduler
         ->or_else(jnt(FOS_ESP_THREAD_STACK_SIZE)) // use default environmental variable
         ->int_value();
   }*/

  static void THREAD_FUNCTIONXX(void *vptr_thread) {
    const auto *fthread = static_cast<Thread *>(vptr_thread);
    if(!fthread) {
      LOG_EXCEPTION(Obj::to_noobj(), fError("unable to acquire thread state"));
      return;
    }
    if(!fthread->thread_obj_) {
      LOG_EXCEPTION(Obj::to_noobj(), fError("unable to acquire thread obj"));
      return;
    }
    if(!fthread->thread_function_) {
      LOG_EXCEPTION(fthread->thread_obj_,
                    fError("unable to acquire thread function: %s", fthread->thread_obj_->toString().c_str()));
      return;
    }
    try {
      FEED_WATCHDOG();
      fthread->thread_function_(fthread->thread_obj_);
      FEED_WATCHDOG();
      vTaskDelete(nullptr);
    } catch(const std::exception &e) {
      LOG_EXCEPTION(fthread->thread_obj_, e);
    }
  }

  Thread::Thread(const Obj_p &thread_obj, const Consumer<Obj_p> &thread_function) :
    thread_obj_(thread_obj), thread_function_(thread_function), handler_(std::make_any<TaskHandle_t *>(nullptr)) {
    int stack_size;
    if(thread_obj->is_rec()) {
      stack_size = thread_obj->rec_get("stack_size") // check provided obj
          ->or_else(thread_obj->rec_get("+/stack_size")->none_one()) // check one depth more (e.g. config/stack_size)
          // ->or_else(this->rec_get("config/def_stack_size") // check default setting in scheduler
          ->or_else(jnt(FOS_ESP_THREAD_STACK_SIZE)) // use default environmental variable
          ->int_value();
    } else {
      stack_size = ///= this->rec_get("config/def_stack_size") // check default setting in scheduler
          jnt(FOS_ESP_THREAD_STACK_SIZE) // use default environmental variable
          ->int_value();
    }
    const BaseType_t threadResult = xTaskCreatePinnedToCore(THREAD_FUNCTIONXX, // Function that should be called
                                                            this->thread_obj_->vid->toString().c_str(),
                                                            // Name of the task (for debugging)
                                                            stack_size, // Stack size (bytes)
                                                            static_cast<void *>(this), // Parameter to pass
                                                            CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, // Task priority
                                                            this->get_handler<TaskHandle_t *>(), // Task handle
                                                            tskNO_AFFINITY); // Processor core
    if(pdPASS != threadResult)
      throw fError("unable to spawn thread: %s", this->thread_obj_->toString().c_str());
  }


  void Thread::halt() {
    vTaskDelete(nullptr);
  }

  void Thread::yield() {
    taskYIELD();
  }

  void Thread::delay(const uint64_t milliseconds) {
    vTaskDelay(milliseconds / portTICK_PERIOD_MS);
  }
}
#endif
