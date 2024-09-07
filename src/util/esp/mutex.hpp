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

#ifndef fhatos_mutex_hpp
#define fhatos_mutex_hpp

#ifdef ESP32
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#endif

//TODO: std::mutex

namespace fhatos {
  template<uint16_t WAIT_TIME_MS = 250>
  class Mutex {
#if defined(ESP32)

    private:
      QueueHandle_t xmutex = xSemaphoreCreateMutex();
      const char* _label;
#endif

  public:
   explicit Mutex(const char *label = "<anon>") : _label(label) {}

#if defined(ESP32)
    ~Mutex() { vSemaphoreDelete(this->xmutex); }
#endif

    template<typename T = void *>
    T lockUnlock(const std::function<T()> criticalFunction,
                 const uint16_t millisecondsWait = WAIT_TIME_MS) const {
#if defined(ESP32)
      if (pdTRUE ==
          xSemaphoreTake(this->xmutex,
                         (TickType_t)(millisecondsWait / portTICK_PERIOD_MS))) {
        T t = criticalFunction();
        if (pdTRUE == xSemaphoreGive(this->xmutex))
          return t;
       }
        throw fError("Unable to lock mutex %s\n", this->_label);
#elif defined(ESP8266)
      return T(criticalFunction());
#endif
    }

    const bool lock(const uint16_t millisecondsWait = WAIT_TIME_MS) {
      return this->xmutex != NULL && pdTRUE ==
                                     xSemaphoreTake(this->xmutex,
                                                    (TickType_t)(millisecondsWait / portTICK_PERIOD_MS));
    }

    const bool unlock() {
      return this->xmutex != NULL && pdTRUE == xSemaphoreGive(this->xmutex);
    }
  };
} // namespace fhatos

#endif
