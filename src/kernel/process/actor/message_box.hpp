#ifndef fhatos_kernel_message_box_hpp
#define fhatos_kernel_message_box_hpp

#include <fhatos.hpp>
#include <kernel/structure/structure.hpp>

namespace fhatos::kernel {

    template<typename T>
    struct MessageBox {
    public:
        [[nodiscard]] virtual const bool push(const T &message) { return true; }

        [[nodiscard]] virtual const uint16_t size() const { return 0; }

    protected:
        virtual Option<T> pop() { return {}; }
    };
} // namespace fhatos::kernel

#endif