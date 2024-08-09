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

#include <list>
#include <util/enums.hpp>
#include <util/options.hpp>

#ifndef FOS_LOGGING
#define FOS_LOGGING INFO
#endif


namespace fhatos {
  enum LOG_TYPE { ALL = 0, TRACE = 1, DEBUG = 2, INFO = 3, WARN = 4, ERROR = 5, NONE = 6 };
  static const Enums<LOG_TYPE> LOG_TYPES = Enums<LOG_TYPE>({{ALL, "ALL"},
                                                            {TRACE, "TRACE"},
                                                            {DEBUG, "DEBUG"},
                                                            {INFO, "INFO"},
                                                            {WARN, "WARN"},
                                                            {ERROR, "ERROR"},
                                                            {NONE, "NONE"}});
  class Logger {
  public:
    static void MAIN_LOG(const LOG_TYPE type, const char *format, ...) {
      if ((uint8_t) type < (uint8_t) Options::singleton()->log_level<LOG_TYPE>())
        return;
      char buffer[512];
      va_list arg;
      va_start(arg, format);
      int length = vsnprintf(buffer, 512, format, arg);
      buffer[length] = '\0';
      va_end(arg);
      if (type == NONE)
        Options::singleton()->printer<>()->print("");
      else if (type == ERROR)
        Options::singleton()->printer()->print("!r[ERROR]!! ");
      else if (type == WARN)
        Options::singleton()->printer()->print("!y[WARN] !! ");
      else if (type == INFO)
        Options::singleton()->printer()->print("!g[INFO] !! ");
      else if (type == DEBUG)
        Options::singleton()->printer()->print("!y[DEBUG]!! ");
      else if (type == TRACE)
        Options::singleton()->printer()->print("!r[TRACE]!! ");
      Options::singleton()->printer()->print(buffer);
    }
  };
} // namespace fhatos
#endif
