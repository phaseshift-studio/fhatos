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

#include <exception>
#include <string>
#include "string_helper.hpp"
#include <memory>

using std::string;
using std::unique_ptr;
using std::make_unique;
namespace fhatos {

  ///////////////////////
  /// EXCEPTION TYPES ///
  ///////////////////////
  class fError final : public std::exception {
  protected:
   string message_;

  public:
    template<typename... Args>
    explicit fError(const char* format, const Args... args) : message_{StringHelper::format(format, args...)} {

    }

    template<typename... Args>
    static fError create(const std::string &type_id, const char * format, const Args... args) noexcept {
      return fError(string("!g[!m").append(type_id).append("!g]!! ").append(format).c_str(), args...);
    }

    [[nodiscard]] virtual const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_USE_NOEXCEPT override {
      return this->message_.c_str();
    }
  };
} // namespace fhatos
#endif
