#ifndef fhatos_kernel__log_hpp
#define fhatos_kernel__log_hpp

#include <kernel/process/actor/actor.hpp>

namespace fhatos::kernel {
    
template <typename LOG_MESSAGE> class Log : public Actor<LOG_MESSAGE> {};
} // namespace fhatos::kernel

#endif