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
#include <memory>

using namespace std;

namespace fhatos {

  class Options final {
  private:
    uint8_t LOGGING = 3; // INFO
    void *ROUTING;
    Ansi<> *PRINTING = Ansi<>::singleton();
    any PARSER;

    explicit Options(){};

  public:
    static Options *singleton() {
      static Options options = Options();
      return &options;
    }

    //////////////////////////
    //////// ROUTING ////////
    template<typename ROUTER>
    ROUTER *router() {
      if (nullptr == ROUTING)
        throw fError("No router secified in global options\n");
      return (ROUTER *) this->ROUTING;
    }
    template<typename ROUTER>
    void router(const ROUTER *router) {
      this->ROUTING = (void *) router;
    }
    //////////////////////////
    //////// LOGGING ////////
    template<typename LOG_LEVEL>
    LOG_LEVEL log_level() {
      return (LOG_LEVEL) this->LOGGING;
    }

    Options *log_level(const uint8_t log_level_enum) {
      this->LOGGING = log_level_enum;
      return this;
    }
    //////////////////////////
    //////// PRINTING ////////
    template<typename PRINTER = Ansi<>>
    PRINTER *printer() {
      if (nullptr == PRINTING)
        throw fError("No printer secified in global options\n");
      return (PRINTER *) this->PRINTING;
    }
    template<typename PRINTER = Ansi<>>
    Options *printer(PRINTER *printer) {
      this->PRINTING = printer;
      return this;
    }
    ////////////////////////
    /////// PARSING ///////
    template<typename OBJ>
    shared_ptr<OBJ> parser(const string &bcode) {
      if (!PARSER.has_value())
        throw fError("No parser specified in global options\n");
      return std::any_cast<function<shared_ptr<OBJ>(string)>>(this->PARSER)(bcode);
    }
    template<typename OBJ>
    void parser(const std::function<shared_ptr<OBJ>(string)> parser) {
      PARSER = any(parser);
    }
    //////////////////////////
  };
} // namespace fhatos
#endif
