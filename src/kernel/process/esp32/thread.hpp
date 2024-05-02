#ifndef fhatos_kernel_thread_hpp
#define fhatos_kernel_thread_hpp

#include <fhatos.hpp>
#include <kernel/process/process.hpp>
#include <kernel/structure/structure.hpp>
//

namespace fhatos::kernel {

class Thread : public Process {

protected:
  bool _running = true;

public:

  TaskHandle_t handle{};
  explicit Thread(const ID &id) : Process(id,THREAD) {}

  void delay(const uint64_t milliseconds) override {
    vTaskDelay(milliseconds / portTICK_PERIOD_MS);
  }

  virtual void loop() override { Process::loop(); }

  void yield() override { taskYIELD(); }

  void stop() override { this->_running = false; }

  [[nodiscard]] bool running() const override { return this->_running; }
};
} // namespace fhatos::kernel

#endif