#ifndef fhatos_process_hpp
#define fhatos_process_hpp

#include <fhatos.hpp>
#include <thread>
#include <structure/furi.hpp>
//

namespace fhatos {
  enum PType { THREAD, FIBER, COROUTINE, KERNEL };

  static const char *P_TYPE_STR(const PType pType) {
    switch (pType) {
      case THREAD:
        return "thread";
      case FIBER:
        return "fiber";
      case COROUTINE:
        return "coroutine";
      case KERNEL:
        return "kernel";
      default:
        return "<unknown process>";
    }
  }

  class Process : public IDed {
  protected:
    bool _running = false;

  public:
    const PType type;

    explicit Process(const ID &id, const PType pType)
      : IDed(id), type(pType) {
    }

    //~Process() { this->stop(); }

    virtual void setup() {
      this->_running = true;
    };

    virtual void loop() {
    }

    virtual void stop() {
      this->_running = false;
    };

    bool running() const { return this->_running; }

    virtual void delay(const uint64_t milliseconds) {
    };

    virtual void yield() {
    };
  };

  class KernelProcess : public Process {
  public:
    explicit KernelProcess(const ID &id) : Process(id, KERNEL) {
    }

    void delay(const uint64_t milliseconds) override {
      std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    void yield() override { std::this_thread::yield(); }
  };
} // namespace fhatos

#endif
