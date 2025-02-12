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

#include <chrono>
#include <thread>
#include "../../../../../fhatos.hpp"
#include "../../../../../lang/obj.hpp"

namespace fhatos {
  class ThreadXX {
  public:
    Obj_p thread_obj;
    std::thread threadxx;

     ThreadXX(const Consumer<Obj_p> function,const Obj_p& thread_obj) : thread_obj(thread_obj), threadxx(std::move(function),thread_obj) {
       }


    void stop()  {

      if(this->threadxx.joinable()) {
        try {
          if(this->threadxx.get_id() != std::this_thread::get_id() /*&& std::this_thread::get_id() == *scheduler_thread*/)
            this->threadxx.join();
          else
            this->threadxx.detach();
        } catch(const std::runtime_error &e) {
          fError::create(this->thread_obj-> toString(), "unable to halt thread: %s", e.what());
        }
      }
    }

    void delay(const uint64_t milliseconds)  {
      std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    void yield()  {
      std::this_thread::yield();
      }

  };
} // namespace fhatos

#endif
