#ifndef fhatos_kernel__process_hpp
#define fhatos_kernel__process_hpp

#include <fhatos.hpp>
#include <kernel/structure/structure.hpp>
//

namespace fhatos::kernel {

    class Process : public IDed {

    public:
        explicit Process(const ID &id) : IDed(id) {}

        virtual void setup() {};

        virtual void loop() {}

        virtual void stop() {};

        [[nodiscard]] virtual bool running() const { return true; }

        virtual void delay(const uint64_t milliseconds) {};

        virtual void yield() {};
    };

    class KernelProcess : public Process {
    public:
        explicit KernelProcess(const ID &id) : Process(id) {}

        void stop() override { this->_running = false; };

        [[nodiscard]] bool running() const override { return this->_running; }

        void delay(const uint64_t milliseconds) override {
            ::delay(milliseconds);
        }

        void yield() override { ::yield(); }

    protected:
        bool _running = true;
    };

} // namespace fhatos::kernel

#endif