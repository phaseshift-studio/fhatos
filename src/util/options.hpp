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
#ifndef fhatos_options_hpp
#define fhatos_options_hpp

#include <any>
#include <functional>
#include <memory>
#include "fhat_error.hpp"

namespace fhatos {
  using std::function;
  using std::any;
  using std::shared_ptr;
  using std::string;
  // TODO: singleton methods in global fhatos namespace for terser syntax
  inline uint8_t LOG_LEVEL = 3; // INFO

  class Options final {
    any printer_{};

    explicit Options() = default;

  public:
    static Options *singleton() {
      static Options options = Options();
      return &options;
    }

    //////////////////////////
    //////// printer_ ////////
    template<typename PRINTER>
    shared_ptr<PRINTER> printer() {
      if(!printer_.has_value())
        throw fError("No printer specified in global options\n");
      return std::any_cast<shared_ptr<PRINTER>>(this->printer_);
    }

    template<typename PRINTER>
    Options *printer(const shared_ptr<PRINTER> &printer) {
      this->printer_ = any(printer);
      return this;
    }
  };
} // namespace fhatos
#endif
