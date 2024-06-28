#ifndef fhatos_ptr_helper
#define fhatos_ptr_helper

// #include <fhatos.hpp>

namespace fhatos {

  class PtrHelper {
  private:
    PtrHelper() = delete;
    template<typename T>
    struct NonDeleter {
      void operator()(const T *ptr) { /*LOG(INFO, "Deleting...not!\n");*/
      }
    };

  public:
    template<typename T>
    static NonDeleter<T> NON_DELETER_SINGLETON() {
      static auto *singleton = new NonDeleter<T>();
      return *singleton;
    }

    template<typename T>
    static ptr<T> no_delete(ptr<T> ptr_t) {
      ptr<T> temp = ptr<T>((T *) nullptr, NON_DELETER_SINGLETON<T>());
      temp.swap(ptr_t);
      return temp;
    }

    template<typename T>
    static ptr<T> no_delete(const T t) {
      return share(t, NON_DELETER_SINGLETON<T>());
    }

    template<typename T>
    static ptr<T> no_delete(T *t) {
      return ptr<T>(t, NON_DELETER_SINGLETON<T>());
    }
  };

} // namespace fhatos
#endif
