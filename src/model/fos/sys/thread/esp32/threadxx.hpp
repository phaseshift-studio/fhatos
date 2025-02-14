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
#ifndef fhatos_threadxx_hpp
#define fhatos_threadxx_hpp

#include "../../../../../fhatos.hpp"
#include "../../../../../lang/obj.hpp"

namespace fhatos {
  class ThreadXX {
  protected:
    int find_stack_size() {
      return ROUTER_READ(this->thread_obj->vid->extend("stack_size")) // check provided obj
          ->or_else(ROUTER_READ(this->thread_obj->vid->extend("+/stack_size"))->none_one())
          // check one depth more (e.g. config/stack_size)
          ->or_else(ROUTER_READ(SCHEDULER_ID->extend("config/def_stack_size")))
          // check default setting in scheduler
          ->or_else(jnt(FOS_ESP_THREAD_STACK_SIZE)) // use default environmental variable
          ->int_value();
    }

  public:
  const Obj* thread_obj;
  const Consumer<Obj_p> thread_function;
   TaskHandle_t* threadxx;

 ThreadXX(const Consumer<Obj_p> function,const Obj_p& thread_obj) : thread_obj(new Obj(*thread_obj)), thread_function(std::move(function)), threadxx(nullptr) {
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
                                               thread_obj->vid->toString().c_str(), // Name of the task (for debugging)
                                               stack_size, // Stack size (bytes)
                                              (void*) this, // Parameter to pass
                                               CONFIG_ESP32_PTHREAD_TASK_PRIO_DEFAULT, // Task priority
                                               this->threadxx, // Task handle
                                               tskNO_AFFINITY); // Processor core
if(pdPASS != threadResult) 
throw fError("unable to spawn thread: %s",this->thread_obj->toString().c_str());
       }


    static void THREAD_FUNCTIONXX(void *vptr_thread) {
      auto *thread = static_cast<ThreadXX *>(vptr_thread);
      if(!thread->thread_obj)
      throw fError("unable to acquire thread obj");
      if(!thread->thread_function)
      throw fError("unable to acquire thread function: %s",thread->thread_obj->toString().c_str());

      const Obj_p o = thread->thread_obj->clone();
      thread->thread_function(o);
      // ESP_ERROR_CHECK(heap_trace_start(HEAP_TRACE_LEAKS));
    /*  try {
        while (!thread->thread_obj->rec_get("halt")->bool_value()) {
          thread->thread_obj->rec_get("loop")->apply(thread->thread_obj);
          FEED_WATCDOG(); // feeds the watchdog for the task
        }
      } catch (fError error) {
        LOG_PROCESS(ERROR, thread->thread_obj, "thread processor error: %s\n", error.what());
      }
       thread->stop();*/
      // ESP_ERROR_CHECK(heap_trace_stop());
      // heap_trace_dump();
    
    }


    void stop()  {
  vTaskDelete(nullptr);
    }
    
    void yield()  {
      taskYIELD();
    }

    void delay(const uint64_t milliseconds) {
      vTaskDelay(milliseconds / portTICK_PERIOD_MS);
    }
  };
} // namespace fhatos

#endif
