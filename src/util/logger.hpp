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
#include <util/enums.hpp>
#include <util/options.hpp>


namespace fhatos {
  enum LOG_TYPE { ALL = 0, TRACE = 1, DEBUG = 2, INFO = 3, WARN = 4, ERROR = 5, NONE = 6 };
  static const auto LOG_TYPES = Enums<LOG_TYPE>({{ALL, "ALL"},
                                                            {TRACE, "TRACE"},
                                                            {DEBUG, "DEBUG"},
                                                            {INFO, "INFO"},
                                                            {WARN, "WARN"},
                                                            {ERROR, "ERROR"},
                                                            {NONE, "NONE"}});

  class Logger {
  public:
    static void MAIN_LOG(const LOG_TYPE type, const char *format, ...) {
      if (static_cast<uint8_t>(type) < static_cast<uint8_t>(Options::singleton()->log_level<LOG_TYPE>()))
        return;
      char buffer[FOS_DEFAULT_BUFFER_SIZE];
      va_list arg;
      va_start(arg, format);
      const size_t length = vsnprintf(buffer, FOS_DEFAULT_BUFFER_SIZE, format, arg);
      if (format[strlen(format) - 1] == '\n')
        buffer[length - 1] = '\n';
      buffer[length] = '\0';
      va_end(arg);
      if (type == NONE)
        printer<>()->print("");
      else if (type == ERROR)
        printer<>()->print("!r[ERROR]!! ");
      else if (type == WARN)
        printer<>()->print("!y[WARN] !! ");
      else if (type == INFO)
        printer<>()->print("!g[INFO] !! ");
      else if (type == DEBUG)
        printer<>()->print("!y[DEBUG]!! ");
      else if (type == TRACE)
        printer<>()->print("!r[TRACE]!! ");
      printer<>()->print(buffer);
    }
  };
} // namespace fhatos
#endif
