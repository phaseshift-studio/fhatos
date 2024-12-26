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
#pragma once
#ifndef fhatos_fhat_error_hpp
#define fhatos_fhat_error_hpp

#include <stdarg.h>
#include <exception>
#include <format>

#define FOS_ERROR_MESSAGE_SIZE 500

namespace fhatos {
  using std::string;
#ifndef NATIVE
  ///////////////////////
  /// EXCEPTION TYPES ///
  ///////////////////////
  class fError : public std::exception {
  protected:
    char _message[FOS_ERROR_MESSAGE_SIZE];

  public:
    explicit fError(const char *format, ...) {
      va_list arg;
      va_start(arg, format);
      int length = vsnprintf(_message, FOS_ERROR_MESSAGE_SIZE, format, arg);
      _message[length] = '\0';
      if (length >= FOS_ERROR_MESSAGE_SIZE) {
        _message[FOS_ERROR_MESSAGE_SIZE - 1] = '\n';
        _message[FOS_ERROR_MESSAGE_SIZE - 2] = '.';
        _message[FOS_ERROR_MESSAGE_SIZE - 3] = '.';
        _message[FOS_ERROR_MESSAGE_SIZE - 4] = '.';
      }
      va_end(arg);
    };

    // ~fError() override { delete _message; }

    [[nodiscard]] virtual const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT override {
      return this->_message;
    }
    ////////////////////////////////
#else

  class /*_LIBCPP_EXCEPTION_ABI _LIBCPP_AVAILABILITY_BAD_ANY_CAST*/ fError final : public std::exception {
  protected:
    const char *_message;

  public:
    template<typename... Args>
    explicit fError(const std::string_view &format, const Args... args) noexcept: _message{
      std::vformat(format, std::make_format_args(args...)).c_str()} {
    }

    template<typename... Args>
    static fError create(const std::string &type_id, const std::string_view &format, const Args... args) noexcept {
      return fError(string("!g[!m").append(type_id).append("!g] ").append(format), args...);
    }

    const char *what() const noexcept override { return this->_message; };
#endif
  };
} // namespace fhatos
#undef FOS_ERROR_MESSAGE_SIZE
#endif
