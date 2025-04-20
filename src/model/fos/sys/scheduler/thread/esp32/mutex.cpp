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
#ifdef ARDUINO
#include "../mutex.hpp"
#include <cstdio>
#include <FreeRTOS.h>
#include <semphr.h>
#include "ext/read_write_lock.hpp"

#define READ_WRITE_LOCK_TYPE ReadWriteLockPreferReader
#define USING_READER_PREFERENCE

namespace fhatos {
      // mutexes can not be used in ISR context
       Mutex::Mutex() : handler_(new READ_WRITE_LOCK_TYPE()) {
        //printer<>()->println("creating mutex");
        }

        void Mutex::lock() {
            //printer<>()->println("locking guarded mutex");
            std::any_cast<READ_WRITE_LOCK_TYPE*>(this->handler_)->WriterLock();
        
        }
         void Mutex::unlock() {
           // printer<>()->println("unlocking guarded mutex");
            std::any_cast<READ_WRITE_LOCK_TYPE*>(this->handler_)->WriterUnlock();
         
        }
         void Mutex::lock_shared() {
           // printer<>()->println("locking shared mutex");
            std::any_cast<READ_WRITE_LOCK_TYPE*>(this->handler_)->ReaderLock();
              
        }
        void Mutex::unlock_shared() {
            //printer<>()->println("unlocking shared mutex");
            std::any_cast<READ_WRITE_LOCK_TYPE*>(this->handler_)->ReaderUnlock();
             
        }
         Mutex::~Mutex() {
            //printer<>()->println("destroying mutex");
            READ_WRITE_LOCK_TYPE* h = std::any_cast<READ_WRITE_LOCK_TYPE*>(this->handler_);

            h->WriterUnlock();
#ifdef USING_READER_PREFERENCE
              vSemaphoreDelete(h->ReadLock);
              vSemaphoreDelete(h->ResourceLock);
#else

              vSemaphoreDelete(h->WriteLock);
              vSemaphoreDelete(h->BlockReadersLock);
#endif
            delete std::any_cast<READ_WRITE_LOCK_TYPE*>(handler_);
       }
  }
#endif
