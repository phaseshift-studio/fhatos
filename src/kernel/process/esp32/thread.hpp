#ifndef fhatos_kernel__thread_hpp
#define fhatos_kernel__thread_hpp

#include <fhatos.hpp>
//
#include <kernel/furi.hpp>
#include <kernel/process/process.hpp>

namespace fhatos::kernel {

class Thread : public Process {

protected:
  bool _running = true;

public:
  TaskHandle_t handle{};
  explicit Thread(const ID &id) : Process(id, THREAD) {}

  virtual void delay(const uint64_t milliseconds) override {
    vTaskDelay(milliseconds / portTICK_PERIOD_MS);
  }

  virtual void yield() override { taskYIELD(); }

  virtual void stop() override { this->_running = false; }

  virtual const bool running() const override { return this->_running; }
};
} // namespace fhatos::kernel

#endif