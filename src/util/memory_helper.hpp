#pragma once
#ifndef fhatos_memory_helper_hpp
#define fhatos_memory_helper_hpp
#include "../fhatos.hpp"
#ifdef ESP_PLATFORM
#include "semphr.h"
#include "esp_expression_with_stack.h"
#endif

namespace fhatos {
  /*
  static thread_local auto CUSTOM_STACK = new std::stack<Obj_p>();

  class MemoryHelper {
  public:
    MemoryHelper() = delete;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////// CUSTOM_STACK /////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////
#ifdef ESP_PLATFORM
    static Obj_p use_custom_stack(const Runnable_p &function, const int stack_size, const bool psram = true) {
      if(stack_size <= 0) {
        function();
      } else {
        int arch_specific_stack_size = stack_size * sizeof(portSTACK_TYPE);
        portSTACK_TYPE *custom_stack = psram ? (portSTACK_TYPE*)ps_malloc(arch_specific_stack_size) : (portSTACK_TYPE*)malloc(arch_specific_stack_size);
        assert(custom_stack != NULL);
        if(!psramInit())
          LOG_WRITE(WARN,Obj::to_noobj().get(), L("!ycustom stack!! could not be created in psram ({} bytes)\n",stack_size));
        //Allocate a mutex to protect its usage:
        SemaphoreHandle_t custom_stack_lock = xSemaphoreCreateMutex();
        assert(custom_stack_lock != NULL);
        ESP_EXECUTE_EXPRESSION_WITH_STACK(custom_stack_lock, custom_stack,arch_specific_stack_size, function);
        vSemaphoreDelete(custom_stack_lock);
        free(custom_stack);
      }
      return Obj::to_noobj();
    }
#else
    static Obj_p use_custom_stack(const Runnable_p &function, const int stack_size, const bool psram = true) {
      //LOG_WRITE(WARN, inst.get(),L("!yusing custom stack !g[!ysize!!:{}!g]!!\n", stack_size));
      function();
      return Obj::to_noobj();
    }
#endif
    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////
  };*/
}
#endif
