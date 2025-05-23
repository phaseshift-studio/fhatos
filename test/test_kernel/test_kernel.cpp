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

#ifndef fhatos_test_kernel_cpp
#define fhatos_test_kernel_cpp

#include "../test_fhatos.hpp"
///////////// COMMON MODELS /////////////
// #include <model/driver/gpio/arduino_gpio_driver.hpp>
// #include <model/driver/i2c/arduino_i2c_master_driver.hpp>
//  #include <model/pin/gpio.hpp>
//  #include <model/pin/interrupt.hpp>
//  #include <model/pin/pwm.hpp>
//////////// ESP SOC MODELS /////////////
#ifdef ESP_ARCH
#include "../../src/model/soc/esp/wifi.hpp"
#include "../../src/model/soc/memory/esp32/memory.hpp"
#include "../../src/util/esp/psram_allocator.hpp"
#endif

namespace fhatos {

  void test_user_boot_loader() {
      // simply does default test_boot_config.obj from test_fhatos.cpp
  }

  FOS_RUN_TESTS( //
      FOS_RUN_TEST(test_user_boot_loader); //
  )
} // namespace fhatos

SETUP_AND_LOOP();
#endif
