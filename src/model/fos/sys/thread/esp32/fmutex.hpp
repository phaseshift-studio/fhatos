#pragma once
#ifdef ARDUINO
#ifndef fhatos_fmutex_hpp
#define fhatos_fmutex_hpp

#include <exception>
#include <string>
#include <cstdio>
#include <FreeRTOS.h>
#include <semphr.h>

namespace fhatos {
  class MutexCreateException : public std::exception {
    public:
        MutexCreateException() {
            sprintf(error_string_, "failed to create mutex");
        }
        virtual const char *what() const throw(){
            return error_string_;
        }
    private:
        char error_string_[80]{};
  };
  // mutexes can not be used in ISR context
  class fMutex {
    public:
        fMutex() {
          this->mutex_ = xSemaphoreCreateMutex();
          if (this->mutex_ == NULL) {
            throw MutexCreateException();
          }
        }
        virtual bool lock() {
          BaseType_t success = xSemaphoreTake(this->mutex_, portMAX_DELAY);
          return success == pdTRUE ? true : false;
        }
        virtual bool unlock() {
          BaseType_t success = xSemaphoreGive(this->mutex_);
          return success == pdTRUE ? true : false;
        }
        virtual bool lock_shared() {
          return this->lock();
        }
        virtual bool unlock_shared() {
          return this->unlock();
        }
        virtual ~fMutex() {
          vSemaphoreDelete(this->mutex_);
        }
    protected:
        SemaphoreHandle_t mutex_;

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
