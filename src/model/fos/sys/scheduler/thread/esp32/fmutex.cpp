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
#include "../fmutex.hpp"
#include <cstdio>
#include <FreeRTOS.h>
#include <semphr.h>

namespace fhatos {
      // mutexes can not be used in ISR context
       fMutex::fMutex() : handler_(xSemaphoreCreateMutex()) {
          if (std::any_cast<SemaphoreHandle_t>(this->handler_) == NULL)
            throw fError("unable to construct mutex");
        }
         void fMutex::lock() {
          BaseType_t success = xSemaphoreTake(std::any_cast<SemaphoreHandle_t>(this->handler_), portMAX_DELAY);
          if(success != pdTRUE) throw fError("unable to lock mutex");
        }
         void fMutex::unlock() {
          BaseType_t success = xSemaphoreGive(std::any_cast<SemaphoreHandle_t>(this->handler_));
          if(success != pdTRUE) throw fError("unable to unlock mutex");
        }
         void fMutex::lock_shared() {
           this->lock();
        }
        void fMutex::unlock_shared() {
           this->unlock();
        }
         fMutex::~fMutex() {
          vSemaphoreDelete(std::any_cast<SemaphoreHandle_t>(this->handler_));
        }
  }
#endif
