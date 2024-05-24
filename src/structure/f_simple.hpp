#ifndef fhatos_f_simple_hpp
#define fhatos_f_simple_hpp

#include <fhatos.hpp>
#include <chrono>
#include <thread>
#include FOS_PROCESS(thread.hpp)

namespace fhatos {
  template<typename PROCESS = Thread>
  class fSimple final : public PROCESS {
  public:
    explicit fSimple(const ID &id = "s1@127.0.0.1"): PROCESS(id) {
      LOG(INFO, "Simple constructor: %s\n", this->id().toString().c_str());
    }

    virtual void setup() override {
      PROCESS::setup();
      LOG(INFO, "Simple setup(): %s\n", this->id().toString().c_str());
    }

    virtual void stop() override {
      LOG(INFO, "Simple stop(): %s\n", this->id().toString().c_str());
      PROCESS::stop();
    }

    auto awake_time(int milliseconds) {
      using std::chrono::operator"" ms;
      return std::chrono::steady_clock::now() + (milliseconds * 1ms);
    }

    virtual void loop() override {
      PROCESS::loop();
      if (this->counter++ == 0) {
        LOG(INFO, "Simple loop() start: %s\n", this->id().toString().c_str());
      }
      std::this_thread::sleep_until(awake_time(2000));
      LOG(INFO, "Simple loop() repeat (%i seconds): %s\n", this->counter * 2, this->id().toString().c_str());
      if (this->counter > 3) {
        LOG(INFO, "Simple loop() end: %s\n", this->id().toString().c_str());
        this->stop();
      }
    }

  private:
    int counter = 0;
  };
} // namespace fhatos

#endif
