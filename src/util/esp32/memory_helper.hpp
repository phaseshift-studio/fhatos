#pragma once
#ifndef fhatos_memory_helper_hpp
#define fhatos_memory_helper_hpp

#include "../../fhatos.hpp"
#include "semphr.h"
#include "esp_expression_with_stack.h"

class MemoryHelper {
  public:
    MemoryHelper() = delete;

  static void use_custom_stack(const int stack_size, const shared_stack_function f) {
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
  }
};

#endif