#pragma once
#ifndef fhatos_error_hpp
#define fhatos_error_hpp

#include "../fhatos.hpp"
#include <stack>
#include "../util/string_helper.hpp"
#include "../furi.hpp"

namespace fhatos {
  class Error {
  public:
    Error* fail_stack;
    string message;
  explicit Error(const string message) : message{message} {
    }

  template <typename ...Args>
  static Error&& create(const fURI& source, const string& format, const Args... args) {
     return std::move(Error(StringHelper::format(format,args...)));
  }
  Error&& add_fail(Error& error) {
      this->fail_stack = &error;
      return std::move(error);
  }
};
}
#endif