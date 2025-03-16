#pragma once
#ifndef fhatos_memory_helper_hpp
#define fhatos_memory_helper_hpp
#include "../fhatos.hpp"
#include "../model/fos/sys/scheduler/scheduler.hpp"
#ifdef ESP_ARCH
#include "semphr.h"
#include "esp_expression_with_stack.h"
#endif

namespace fhatos {
  class MemoryHelper {
  public:
    MemoryHelper() = delete;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////// CUSTOM_STACK /////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////
#ifdef ESP_ARCH
  static void use_custom_stack(const Runnable_p f,const int stack_size) {
    LOG_OBJ(INFO,fScheduler::singleton(), "!ytemporary stack!! created (%i bytes)\n",stack_size);
    //Allocate a stack buffer, from heap or as a static form:
    int arch_specific_stack_size = stack_size * sizeof(portSTACK_TYPE);
    portSTACK_TYPE *custom_stack = (portSTACK_TYPE*)malloc(arch_specific_stack_size);
    assert(custom_stack != NULL);
    //Allocate a mutex to protect its usage:
    SemaphoreHandle_t custom_stack_lock = xSemaphoreCreateMutex();
    assert(custom_stack_lock != NULL);
    ESP_EXECUTE_EXPRESSION_WITH_STACK(custom_stack_lock, custom_stack,arch_specific_stack_size,f);
    vSemaphoreDelete(custom_stack_lock);
    free(custom_stack);
     LOG_OBJ(INFO,fScheduler::singleton(),"!ytemporary stack!! destroyed (%i bytes)\n",stack_size);
  }
#else
    inline static void use_custom_stack(const Runnable_p f, const int) {
      f();
    }
#endif
    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////
  };
}
#endif
