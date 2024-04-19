#ifndef fhatos_kernel__mutex_hpp
#define fhatos_kernel__mutex_hpp

#include <fhatos.hpp>
#include <freertos/queue.h>

namespace fhatos::kernel {

class Mutex {
#if defined(ESP32)
private:
  QueueHandle_t xmutex = xSemaphoreCreateMutex();
#endif

public:
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
};
} // namespace fhatos::kernel

#endif