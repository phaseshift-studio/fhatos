#ifndef fhatos_thread_hpp
#define fhatos_thread_hpp

#include <fhatos.hpp>
//
#include FOS_PROCESS(process.hpp)

namespace fhatos {
  class Thread : public Process {
  public:
    TaskHandle_t handle;

    explicit Thread(const ID &id) : Process(id, THREAD) {
    }

    void setup() override {
      Process::setup();
    }

    void stop() override {
      Process::stop();
    }

    void delay(const uint64_t milliseconds) override {
      vTaskDelay(milliseconds / portTICK_PERIOD_MS);
    }

    void yield() override { taskYIELD(); }
  };
} // namespace fhatos

#endif
