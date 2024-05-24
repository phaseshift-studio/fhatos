#ifndef fhatos_fhat_error_hpp
#define fhatos_fhat_error_hpp

#include <exception>
#include <functional>

namespace fhatos {
#ifndef NATIVE
  ///////////////////////
  /// EXCEPTION TYPES ///
  ///////////////////////
  class fError : public std::exception {
  protected:
    char *_message;

  public:
    explicit fError(const char *format, ...) {
      va_list arg;
      va_start(arg, format);
      char temp[64];
      _message = temp;
      size_t len = vsnprintf(temp, sizeof(temp), format, arg);
      va_end(arg);
      if (len > sizeof(temp) - 1) {
        _message = new(std::nothrow) char[len + 1];
        if (!_message) {
          return;
        }
        va_start(arg, format);
        vsnprintf(_message, len + 1, format, arg);
        va_end(arg);
      }
      if (_message != temp) {
        delete[] _message;
      }
    };

    // ~fError() override { delete _message; }

    [[nodiscard]]
    virtual const char *
    what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT override {
      return this->_message;
    }
  };
}

////////////////////////////////
#else
 class fError : std::exception {
 protected:
   char *_message;

 public:
   explicit fError(const char *format, ...) noexcept {
     va_list arg;
     va_start(arg, format);
     char temp[64];
     _message = temp;
     size_t len = vsnprintf(temp, sizeof(temp), format, arg);
     va_end(arg);
     if (len > sizeof(temp) - 1) {
       _message = new(std::nothrow) char[len + 1];
       if (!_message) {
         return;
       }
       va_start(arg, format);
       vsnprintf(_message, len + 1, format, arg);
       va_end(arg);
     }
     if (_message != temp) {
       delete[] _message;
     }
   };

   // ~fError() override { delete _message; }

   const char *what() const noexcept {
     return this->_message;
   }
 };
 }
#endif
#endif
