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
#ifndef fhatos_logger_hpp
#define fhatos_logger_hpp
#include "enums.hpp"
#include <mutex>
#include "options.hpp"


namespace fhatos {
  //inline auto stdout_mutex = std::mutex();

  enum LOG_TYPE { ALL = 0, TRACE = 1, DEBUG = 2, INFO = 3, WARN = 4, ERROR = 5, NONE = 6, OFF = 7 };

  static const auto LOG_TYPES = Enums<LOG_TYPE>({{ALL, "ALL"},
    {TRACE, "TRACE"},
    {DEBUG, "DEBUG"},
    {INFO, "INFO"},
    {WARN, "WARN"},
    {ERROR, "ERROR"},
    {NONE, "NONE"},
    {OFF, "OFF"}});

  class Logger {
  public:
    template<typename... Args>
    static void MAIN_LOG(const LOG_TYPE type, const char *format, const Args... args) {
      if(static_cast<uint8_t>(type) < LOG_LEVEL)
        return;
      // control garbled concurrent writes (destructor releases lock)
      //std::lock_guard<std::mutex> lock(stdout_mutex);
      if(type == NONE)
        printer<>()->print("");
      else if(type == ERROR)
        printer<>()->print("!r[ERROR]!! ");
      else if(type == WARN)
        printer<>()->print("!y[WARN] !! ");
      else if(type == INFO)
        printer<>()->print("!g[INFO] !! ");
      else if(type == DEBUG)
        printer<>()->print("!y[DEBUG]!! ");
      else if(type == TRACE)
        printer<>()->print("!r[TRACE]!! ");
      printer<>()->print(StringHelper::format(format, args...).c_str());
    }
  };
} // namespace fhatos
#endif
