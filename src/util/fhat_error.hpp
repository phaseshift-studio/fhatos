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
#include <string>

#define FOS_ERROR_MESSAGE_SIZE 500

namespace fhatos {
  using std::string;
#ifndef NATIVE
  ///////////////////////
  /// EXCEPTION TYPES ///
  ///////////////////////
  class fError : public std::exception {
  protected:
    std::unique_ptr<const char *> message_;

  public:
    template<typename... Args>
    explicit fError(const std::string_view &format, const Args... args) : message_{
      std::make_unique<const char *>(strdup(std::vformat(format, std::make_format_args(args...).c_str())))
    };

    template<typename... Args>
    static fError create(const std::string &type_id, const std::string_view &format, const Args... args) noexcept {
      return fError(string("!g[!m").append(type_id).append("!g] ").append(format), args...);
    }

    [[nodiscard]] virtual const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT override {
      return *this->_message;
    }

    ////////////////////////////////
#else

  class /*_LIBCPP_EXCEPTION_ABI _LIBCPP_AVAILABILITY_BAD_ANY_CAST*/ fError final : public std::exception {
  protected:
      std::unique_ptr<const char*> message_;

  public:
    template<typename... Args>
    explicit fError(const std::string_view &format, const Args... args) noexcept: message_{
      std::make_unique<const char*>(strdup(std::vformat(format, std::make_format_args(args...)).c_str()))} {
    }

    template<typename... Args>
    static fError create(const std::string &type_id, const std::string_view &format, const Args... args) noexcept {
      return fError(string("!g[!m").append(type_id).append("!g] ").append(format), args...);
    }

    const char *what() const noexcept override { return *this->message_; };
#endif
  };
} // namespace fhatos
#undef FOS_ERROR_MESSAGE_SIZE
#endif
