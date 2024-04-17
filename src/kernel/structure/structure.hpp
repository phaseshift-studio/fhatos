#ifndef fhat_kernel__structure_hpp
#define fhat_kernel__structure_hpp

#include <exception>

namespace fhatos {
namespace kernel {

template <typename WHAT> class ferror : public std::exception {
public:
  static_assert(std::is_base_of<String, WHAT>::value),
  const char *what() const throw() {
    return WHAT; }
};

} // namespace kernel
} // namespace fhat

#endif