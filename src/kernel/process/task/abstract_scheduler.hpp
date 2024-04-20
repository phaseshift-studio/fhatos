#ifndef fhatos_kernel__abstract_scheduler_hpp
#define fhatos_kernel__abstract_scheduler_hpp

namespace fhatos::kernel {

template <typename THREAD, typename FIBER> class AbstractScheduler {

public:
  virtual bool addThread(THREAD &thread);
  virtual bool addFiber(FIBER &fiber);
  virtual void setup();
  virtual void loop();
};

} // namespace fhatos::kernel

#endif