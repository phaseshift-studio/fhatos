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

#include <map>
#include <string>
#include <util/options.hpp>

#ifndef FOS_LOGGING
#define FOS_LOGGING ERROR
#endif


namespace fhatos {

  enum LOG_TYPE { NONE = 0, DEBUG_MORE = 1, DEBUG = 2, INFO = 3, ERROR = 4 };
  static const std::map<std::string, LOG_TYPE> STR_LOGTYPE = {{{"DEBUG_MORE", LOG_TYPE::DEBUG_MORE},
                                                               {"DEBUG", LOG_TYPE::DEBUG},
                                                               {"INFO", LOG_TYPE::INFO},
                                                               {"ERROR", LOG_TYPE::ERROR},
                                                               {"NONE", LOG_TYPE::NONE}}};
  static const std::map<LOG_TYPE, std::string> LOGTYPE_STR = {{{LOG_TYPE::DEBUG_MORE, "DEBUG_MORE"},
                                                               {LOG_TYPE::DEBUG, "DEBUG"},
                                                               {LOG_TYPE::INFO, "INFO"},
                                                               {LOG_TYPE::ERROR, "ERROR"},
                                                               {LOG_TYPE::NONE, "NONE"}}};


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
        GLOBAL_OPTIONS->printer()->print("");
      else if (type == ERROR)
        GLOBAL_OPTIONS->printer()->print("!r[ERROR]!!  ");
      else if (type == INFO)
        GLOBAL_OPTIONS->printer()->print("!g[INFO]!!  ");
      else if (type == DEBUG)
        GLOBAL_OPTIONS->printer()->print("!y[DEBUG]!!  ");
      else if (type == DEBUG_MORE)
        GLOBAL_OPTIONS->printer()->print("!y[DEBUG_MORE]!!  ");
      GLOBAL_OPTIONS->printer()->print(buffer);
      if (buffer != temp) {
        delete[] buffer;
      }
    }
  };
} // namespace fhatos
#endif
