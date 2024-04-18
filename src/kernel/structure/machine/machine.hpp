#ifndef fhatos_kernel__machine_hpp
#define fhatos_kernel__machine_hpp

namespace fhatos::kernel {

template <typename MACHINE> class Machine {

public:
  Machine() { static_assert(std::is_base_of<Machine, MACHINE>::value); }

  template <typename MACHINE> static MACHINE *singleton() {
    static_assert(std::is_base_of<Machine, MACHINE>::value);
    static MACHINE machine = MACHINE();
    return &machine;
  }
};
} // namespace fhatos::kernel

#endif