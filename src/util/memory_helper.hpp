#pragma once
#ifndef fhatos_memory_helper_hpp
#define fhatos_memory_helper_hpp
#include "../fhatos.hpp"
#ifdef ESP_PLATFORM
#include "semphr.h"
#include "esp_expression_with_stack.h"
#endif

namespace fhatos {
  static thread_local auto CUSTOM_STACK = new std::stack<Obj_p>();

  class MemoryHelper {
  public:
    MemoryHelper() = delete;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////// CUSTOM_STACK /////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////
#ifdef ESP_PLATFORM
  static Obj_p use_custom_stack(const Inst_p& inst,const Obj_p& lhs, const int stack_size,const bool psram = true) {
    //Allocate a stack buffer, from heap or as a static form:
    int arch_specific_stack_size = stack_size * sizeof(portSTACK_TYPE);
    portSTACK_TYPE *custom_stack = psram ? (portSTACK_TYPE*)ps_malloc(arch_specific_stack_size) : (portSTACK_TYPE*)malloc(arch_specific_stack_size);
    assert(custom_stack != NULL);
   /* if(psramInit())
      LOG_WRITE(DEBUG,Scheduler::singleton().get(), L("!ycustom stack!! created in psram ({} bytes)\n",stack_size));
    else
      LOG_WRITE(WARN,Scheduler::singleton().get(), L("!ycustom stack!! could not be created in psram ({} bytes)\n",stack_size));*/
    CUSTOM_STACK->push(inst);
    CUSTOM_STACK->push(lhs);
    //Allocate a mutex to protect its usage:
    SemaphoreHandle_t custom_stack_lock = xSemaphoreCreateMutex();
    assert(custom_stack_lock != NULL);
    ESP_EXECUTE_EXPRESSION_WITH_STACK(custom_stack_lock, custom_stack,arch_specific_stack_size,[]() {
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
#else
    static Obj_p use_custom_stack(const Inst_p &inst, const Obj_p &lhs, const int, const bool psram = true) {
      return std::holds_alternative<Obj_p>(inst->inst_f())
               ? std::get<Obj_p>(inst->inst_f())->apply(lhs)
               : (*std::get<Cpp_p>(inst->inst_f()))(lhs, inst->inst_args()->clone());

    }
#endif
    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////
  };
}
#endif
