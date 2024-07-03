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

#ifndef fhatos_logger_hpp
#define fhatos_logger_hpp

#include <list>
#include <string>
#include <util/enums.hpp>
#include <util/options.hpp>

#ifndef FOS_LOGGING
#define FOS_LOGGING ERROR
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
      if ((uint8_t) type < (uint8_t) GLOBAL_OPTIONS->logger<LOG_TYPE>())
        return;
      va_list arg;
      va_start(arg, format);
      char temp[128];
      char *buffer = temp;
      const size_t len = vsnprintf(temp, sizeof(temp), format, arg);
      va_end(arg);
      if (len > sizeof(temp) - 1) {
        buffer = new (std::nothrow) char[len + 1];
        if (!buffer) {
          return;
        }
        va_start(arg, format);
        vsnprintf(buffer, len + 1, format, arg);
        va_end(arg);
      }
      if (type == NONE)
        GLOBAL_OPTIONS->printer<>()->print("");
      else if (type == ERROR)
        GLOBAL_OPTIONS->printer()->print("!r[ERROR]!!  ");
      else if (type == INFO)
        GLOBAL_OPTIONS->printer()->print("!g[INFO]!!  ");
      else if (type == DEBUG)
        GLOBAL_OPTIONS->printer()->print("!y[DEBUG]!!  ");
      else if (type == TRACE)
        GLOBAL_OPTIONS->printer()->print("!y[TRACE]!!  ");
      GLOBAL_OPTIONS->printer()->print(buffer);
      if (buffer != temp) {
        delete[] buffer;
      }
    }
  };
} // namespace fhatos
#endif
