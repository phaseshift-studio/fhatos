#ifndef fhatos_kernel__process_hpp
#define fhatos_kernel__process_hpp

#include <fhatos.hpp>
#include <kernel/furi.hpp>
//

namespace fhatos::kernel {

class Process : public IDed {

protected:
  bool _running = false;

public:
  const bool parent;
  enum Type { THREAD, FIBER, COROUTINE, KERNEL };
  const Type pType{};

  explicit Process(const ID &id, const Type pType, const bool parent = false)
      : IDed(id), pType(pType), parent(parent) {}

  //~Process() { this->stop(); }

  virtual void setup() { this->_running = true; };

  virtual void loop() {}

  virtual void stop() { this->_running = false; };

  const bool running() const { return this->_running; }

  virtual void delay(const uint64_t milliseconds) {};

  virtual void yield() {};
};

template <typename PROCESS> class ParentProcess {

protected:
  List<PROCESS *> _children;

public:
  const void loopChildren() {
    for (PROCESS *child : this->_children) {
      if (!child->running()) {
        delete child;
      } else {
        child->loop();
      }
    }
  }
  const uint8_t searchChildren(
      const Pattern &childPattern,
      const Consumer<PROCESS *> onFind = [](PROCESS *proc) {},
      const uint8_t depth = 1) {
    if (0 == depth)
      return 0;
    uint8_t counter = 0;
    for (PROCESS *child : this->_children) {
      if (child->id().matches(childPattern)) {
        counter++;
        onFind(child);
        if (depth > 1 && child->parent) {
          counter +=
              reinterpret_cast<ParentProcess<PROCESS> *>(child)->searchChildren(
                  childPattern, onFind, depth - 1);
        }
      }
    }
    return counter;
  }
  const bool spawnChild(const PROCESS *child) { return false; }
  const bool destroyChildren(const Pattern &childPattern = Pattern("#")) {
    this->_children.remove_if([this](PROCESS *child) {
      child->stop();
      delete child;
      return true;
    });
    return true;
  }
  const void children(List<PROCESS *> *aggregator, const uint8_t depth = 1) {
    if (0 == depth)
      return;
    if (1 == depth)
      aggregator->insert(aggregator->end(), _children.begin(), _children.end());
    for (const auto &child : this->_children) {
      aggregator->push_front(child);
      if (child->parent) {
        reinterpret_cast<ParentProcess<PROCESS> *>(child)->children(aggregator,
                                                                    depth - 1);
      }
    }
  }
};

class KernelProcess : public Process {
public:
  explicit KernelProcess(const ID &id) : Process(id, KERNEL) {}

  void delay(const uint64_t milliseconds) override { ::delay(milliseconds); }

  void yield() override { ::yield(); }
};

} // namespace fhatos::kernel

#endif