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
#include <util/fhat_error.hpp>

namespace fhatos {
  using std::function;
  using std::any;
  using std::shared_ptr;
  using std::string;
  // TODO: singleton methods in global fhatos namespace for terser syntax

  inline function<void()> FEED_WATCDOG = []() {
  };

  class Options final {
  private:
    uint8_t log_level_ = 3; // INFO
    any router_{};
    any scheduler_{};
    any system_{};
    any printer_{};
    any parser_{};
    any processor_{};

    explicit Options() = default;

  public:
    static Options *singleton() {
      static Options options = Options();
      return &options;
    }

    //////////////////////////
    //////// SCHEDULING ////////
    template<typename SCHEDULER>
    shared_ptr<SCHEDULER> scheduler() {
      if (!scheduler_.has_value())
        throw fError("No scheduler specified in global options\n");
      return std::any_cast<shared_ptr<SCHEDULER>>(this->scheduler_);
    }

    template<typename SCHEDULER>
    void scheduler(const shared_ptr<SCHEDULER> scheduler) {
      this->scheduler_ = any(scheduler);
    }

    template<typename SYSTEM>
    shared_ptr<SYSTEM> system() {
      if (!this->system_.has_value())
        throw fError("No system specified in global options\n");
      return std::any_cast<shared_ptr<SYSTEM>>(this->system_);
    }

    template<typename SYSTEM>
    void system(const shared_ptr<SYSTEM> system) {
      this->system_ = any(system);
    }

    //////////////////////////
    //////// router_ ////////


    template<typename ROUTER>
    shared_ptr<ROUTER> router() {
      if (!router_.has_value())
        throw fError("No router specified in global options\n");
      return std::any_cast<shared_ptr<ROUTER>>(this->router_);
    }

    template<typename ROUTER>
    void router(const shared_ptr<ROUTER> &router) {
      this->router_ = any(router);
    }

    //////////////////////////
    //////// log_level_ ////////
    template<typename LOG_LEVEL>
    LOG_LEVEL log_level() {
      return static_cast<LOG_LEVEL>(this->log_level_);
    }

    Options *log_level(const uint8_t log_level_enum) {
      this->log_level_ = log_level_enum;
      return this;
    }

    //////////////////////////
    //////// printer_ ////////
    template<typename PRINTER>
    shared_ptr<PRINTER> printer() {
      if (!printer_.has_value())
        throw fError("No printer specified in global options\n");
      return std::any_cast<shared_ptr<PRINTER>>(this->printer_);
    }

    template<typename PRINTER>
    Options *printer(const shared_ptr<PRINTER> &printer) {
      this->printer_ = any(printer);
      return this;
    }

    ////////////////////////
    /////// PARSING ///////
    template<typename OBJ>
    shared_ptr<OBJ> parser(const string &bcode) {
      if (!parser_.has_value())
        throw fError("No parser specified in global options\n");
      return std::any_cast<function<shared_ptr<OBJ>(string)>>(this->parser_)(bcode);
    }

    template<typename OBJ>
    Options *parser(const std::function<shared_ptr<OBJ>(string)> &parser) {
      this->parser_ = any(parser);
      return this;
    }
  };
} // namespace fhatos
#endif