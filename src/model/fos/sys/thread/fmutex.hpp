#pragma once
#ifndef fhatos_fmutex_hpp;
#define fhatos_fmutex_hpp;
/*
#include "../../../../../fhatos.hpp"
#include <exception>
#include <mutex>
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
    fMutex();
    virtual bool lock_shared();
    virtual bool unlock_shared();
    virtual bool lock();
    virtual bool unlock();
  };

  class LockGuard {
  public:
    explicit LockGuard(fMutex &m) :
      mutex_(&m) {
    }
    ~LockGuard() {
      this->mutex_->unlock();
    }
    LockGuard(const LockGuard &) = delete;
  private:
    fMutex *mutex_;
  };
}*/
#endif
