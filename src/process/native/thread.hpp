#ifndef fhatos_thread_hpp
#define fhatos_thread_hpp

#include <fhatos.hpp>
#include <chrono>
#include <thread>
//
#include FOS_PROCESS(process.hpp)

namespace fhatos {
  class Thread : public Process {
  public:
    std::thread *xthread;

    explicit Thread(const ID &id) : Process(id, THREAD) {
    }

    ~Thread() {
      delete this->xthread;
    }

    void setup() override {
      Process::setup();
    }

    void stop() override {
      Process::stop();
    }

    void loop() override {
      Process::loop();
    }

    void delay(const uint64_t milliseconds) override {
      std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    void yield() override {
      std::this_thread::yield();
    }
  };
} // namespace fhatos

#endif
