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

#ifdef ESP_PLATFORM
#include "../memory.hpp"
#include <semphr.h>
#include <esp_expression_with_stack.h>

namespace fhatos {
  static thread_local auto CUSTOM_STACK = new std::stack<Obj_p>();

  Rec_p Memory::main_memory() const {
    return Obj::to_rec({{"total", jnt(ESP.getHeapSize())},
                        {"free", jnt(ESP.getFreeHeap())},
                        {"used", real(static_cast<float>(ESP.getHeapSize()) == 0
                                        ? 0.0f
                                        : (100.0f * (1.0f - (static_cast<float>(ESP.getFreeHeap()) / static_cast<
                                                               float>(ESP.
                                                               getHeapSize())))), REAL_FURI)}});
  }

  Rec_p Memory::inst_memory() const {
    return Obj::to_rec({{"total", jnt(ESP.getSketchSize() + ESP.getFreeSketchSpace())},
                        {"free", jnt(ESP.getFreeSketchSpace())},
                        {"used", real(ESP.getSketchSize() == 0
                                        ? 0.0f
                                        : (100.0f * (1.0f - (static_cast<float>(ESP.getFreeSketchSpace()) /
                                                             static_cast<float>(
                                                               ESP.getSketchSize() + ESP.getFreeSketchSpace())))),
                                      REAL_FURI)}});
  }

  Rec_p Memory::psram_memory() const {
    return Obj::to_rec({{"total", jnt(ESP.getPsramSize())},
                        {"free", jnt(ESP.getFreePsram())},
                        {"used",
                         real(static_cast<float>(ESP.getPsramSize()) == 0
                                ? 0.0f
                                : (100.0f * (1.0f - (static_cast<float>(ESP.getFreePsram()) / static_cast<float>(ESP.
                                                       getPsramSize())))), REAL_FURI)}});
  }

  Rec_p Memory::high_water_mark() const {
    const int free = FOS_ESP_THREAD_STACK_SIZE - uxTaskGetStackHighWaterMark(nullptr);
    return Obj::to_rec({{"total", jnt(FOS_ESP_THREAD_STACK_SIZE)},
                        {"min_free", jnt(free)},
                        {"used", real(FOS_ESP_THREAD_STACK_SIZE == 0
                                        ? 0.0f
                                        : (100.0f * (1.0f - static_cast<float>(free) / static_cast<float>(
                                                       FOS_ESP_THREAD_STACK_SIZE))), REAL_FURI)}});
  }

  Rec_p Memory::cpu_frequency() const {
    return Obj::to_rec({{"freq", jnt(ESP.getCpuFreqMHz())}});
  }

  Obj_p Memory::internal_use_custom_stack(const Inst_p &inst, const Obj_p &lhs, const int stack_size) {
    if(stack_size <= 0)
      return std::holds_alternative<Obj_p>(inst->inst_f())
               ? std::get<Obj_p>(inst->inst_f())->apply(lhs)
               : (*std::get<Cpp_p>(inst->inst_f()))(lhs, inst->inst_args()->clone());
    /////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////
    //Allocate a stack buffer, from heap or as a static form:
    //LOG_WRITE(WARN,inst.get(),L("!yusing custom stack !g[!ysize!!:{}!g]!!\n",stack_size));
    const unsigned int arch_specific_stack_size = stack_size * sizeof(portSTACK_TYPE);
    auto *custom_stack = static_cast<uint8_t *>(ps_malloc(arch_specific_stack_size));
    if(!custom_stack || !psramInit())
      throw fError::create(this->vid->toString(),
                           "!ycustom stack!! allocation failed in psram (%s bytes)",
                           stack_size);
    CUSTOM_STACK->push(inst);
    CUSTOM_STACK->push(lhs);
    //Allocate a mutex to protect its usage:
    const SemaphoreHandle_t custom_stack_lock = xSemaphoreCreateMutex();
    assert(custom_stack_lock != nullptr);
    ESP_EXECUTE_EXPRESSION_WITH_STACK(custom_stack_lock,
                                      custom_stack,
                                      arch_specific_stack_size,
                                      []() {
                                      const Obj_p lhs = CUSTOM_STACK->top();
                                      CUSTOM_STACK->pop();
                                      const Inst_p inst = CUSTOM_STACK->top();
                                      CUSTOM_STACK->pop();
                                      const Obj_p rhs = std::holds_alternative<Obj_p>(inst->inst_f())
                                      ? std::get<Obj_p>(inst->inst_f())->apply(lhs)
                                      : (*std::get<Cpp_p>(inst->inst_f()))(lhs, inst->inst_args()->clone());
                                      CUSTOM_STACK->push(rhs);
                                      });
    vSemaphoreDelete(custom_stack_lock);
    free(custom_stack);
    const Obj_p rhs = CUSTOM_STACK->top();
    CUSTOM_STACK->pop();
    return rhs;
    //LOG_WRITE(DEBUG,Scheduler::singleton().get(),L("!ytemporary stack!! destroyed ({} bytes)\n",stack_size));
  }
}
#endif
