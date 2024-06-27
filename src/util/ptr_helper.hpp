#ifndef fhatos_ptr_helper
#define fhatos_ptr_helper

// #include <fhatos.hpp>

namespace fhatos {
  template<typename T>
  struct NonDeleter {
    void operator()(const T *ptr) { /*LOG(INFO, "Deleting...not!\n");*/
    }
  };
} // namespace fhatos
#endif
