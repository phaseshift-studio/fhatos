#pragma once
#ifdef NATIVE
#ifndef fhatos_fxmutex_hpp;
#define fhatos_fxmutex_hpp;
#include <exception>
#include <shared_mutex>
#include <string>


namespace fhatos {
  using namespace std;
  class MutexCreateException final : public std::exception {
  public:
    MutexCreateException() {
      sprintf(error_string_, "failed to construct mutex");
    }
    [[nodiscard]] const char *what() const throw() override {
      return error_string_;
    }
  private:
    char error_string_[80]{};
  };
  // calling lock twice from the same thread will deadlock
  class fMutex {
  protected:
    std::shared_mutex mutex_;
  public:
    virtual ~fMutex() = default;
    fMutex() : mutex_(std::shared_mutex()) {
    }
    virtual bool lock_shared() {
      this->mutex_.lock_shared();
      return true;
    }
    virtual bool unlock_shared() {
      this->mutex_.unlock_shared();
      return true;
    }
    virtual bool lock() {
       this->mutex_.lock();
      return true;
    }
    virtual bool unlock()  {
      this->mutex_.unlock();
      return true;
    }
  };

  class LockGuard {
  public:
    explicit LockGuard(fMutex& m)  : mutex_(m){
      mutex_.lock();
    }
    ~LockGuard(){
      mutex_.unlock();
    }
  private:
    LockGuard(const LockGuard&) = delete;
    fMutex& mutex_;
  };
}
#endif
#endif
