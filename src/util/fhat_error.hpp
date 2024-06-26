/*******************************************************************************
 FhatOS: A Distributed Operating System
 Copyright (c) 2024 PhaseShift Studio, LLC

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU Affero General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Affero General Public License for more details.

 You should have received a copy of the GNU Affero General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

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
        _message = new (std::nothrow) char[len + 1];
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

    [[nodiscard]] virtual const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT override {
      return this->_message;
    }
  };
}

////////////////////////////////
#else
  class _LIBCPP_EXCEPTION_ABI _LIBCPP_AVAILABILITY_BAD_ANY_CAST fError final : public std::exception {
  protected:
    char _message[250];

  public:
    template<typename... Args>
    explicit fError(const char *format, Args... args) noexcept {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
      sprintf(_message, format, args...);
#pragma clang diagnostic pop
    };
    const char *what() const _NOEXCEPT override { return this->_message; }
  };
}
#endif
#endif
