#ifndef fhatos_kernel_coroutine_hpp
#define fhatos_kernel_coroutine_hpp

#include <fhatos.hpp>
#include <kernel/process/process.hpp>
#include <kernel/structure/structure.hpp>
//

namespace fhatos::kernel {
    class Coroutine : public Process {
    public:
        explicit Coroutine(const ID &id) : Process(id,COROUTINE) {}

        void delay(const uint64_t milliseconds) override {
            // do nothing
        }

        void yield() override {
            // do nothing
        }

        void loop() override {
            // do nothing
        }
    };
} // namespace fhatos::kernel

#endif