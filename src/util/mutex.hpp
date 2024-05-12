#ifndef fhatos_mutex_hpp
#define fhatos_mutex_hpp

#include <fhatos.hpp>
#ifdef ESP32
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#endif

//TODO: std::mutex

namespace fhatos {

class Mutex {
#if defined(ESP32)
private:
  QueueHandle_t xmutex = xSemaphoreCreateMutex();
#endif

public:
#if defined(ESP32)
  ~Mutex() { vSemaphoreDelete(this->xmutex); }
#endif
  template <typename T = void *>
  T lockUnlock(const Supplier<T> criticalFunction,
               const uint16_t millisecondsWait = 250) const {
#if defined(ESP32)
    if (pdTRUE ==
        xSemaphoreTake(this->xmutex,
                       (TickType_t)(millisecondsWait / portTICK_PERIOD_MS))) {
      T t = criticalFunction();
      if (pdTRUE == xSemaphoreGive(this->xmutex))
        return t;
      else
        throw fError("Unable to unlock mutex: %i", __LINE__);
    } else {
      throw fError("Unable to lock mutex: %i", __LINE__);
    }
#elif defined(ESP8266)
    return T(criticalFunction());
#endif
  }

  const bool lock(const uint16_t millisecondsWait = 250) {
    return this->xmutex != NULL && pdTRUE ==
           xSemaphoreTake(this->xmutex,
                          (TickType_t)(millisecondsWait / portTICK_PERIOD_MS));
  }

  const bool unlock(const uint16_t millisecondsWait = 250) {
    
    return this->xmutex != NULL && pdTRUE == xSemaphoreGive(this->xmutex);
  }
};
} // namespace fhatos

#endif