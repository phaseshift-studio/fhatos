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
#ifndef fhatos_gpio_hpp
#define fhatos_gpio_hpp
//
#include "../../../../fhatos.hpp"
#include "../../../../lang/obj.hpp"
#include "../../../../util/obj_helper.hpp"
#include "../../sys/typer/typer.hpp"
#include "../../sys/scheduler/thread/mutex.hpp"
//
#ifdef ARDUINO
#include <Arduino.h>
#elif defined(RASPBERRYPI)
#include <wiringPi.h>
#elif defined(NATIVE)
#include <gpiod.h>
#endif

namespace fhatos {
  static ID_p GPIO_FURI = id_p("/fos/io/gpio");
  static auto GPIO_CHIP_NAME = "gpiochip0";
  static auto gpio_mutex = Mutex();

  class GPIO final {

  public:
    static void register_module() {
      const ID module_id = Typer::singleton()->vid->add_component(*GPIO_FURI);
      REGISTERED_MODULES->insert_or_assign(*GPIO_FURI,
          InstBuilder::build(module_id)
              ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {1, 1})
              ->inst_f([](const Obj_p &, const InstArgs &) {
             //   return ([]() -> Rec_p {
                  return Obj::to_rec(
                      {{vri(GPIO_FURI), Obj::to_type(INT_FURI)},
                       {vri(GPIO_FURI->add_component("scan")),
                        InstBuilder::build(GPIO_FURI->add_component("scan"))
                            ->domain_range(OBJ_FURI, {0, 1}, REC_FURI, {1, 1})
                            ->inst_args(rec({{"chip_name", vri("gpiochip0")}}))
                            ->inst_f([](const Obj_p &, const InstArgs &args) {
                              const Rec_p pins = Obj::to_rec();
#ifdef NATIVE
                              const fURI chip_name = args->arg("chip_name")->uri_value();
                              if(gpiod_chip *chip = gpiod_chip_open_by_name(chip_name.toString().c_str()); !chip) {
                                throw fError(GPIO_FURI->toString().c_str(), "unable to access gpio chip %s\n",
                                             chip_name.toString().c_str());
                              } else {
                                for(int i = 0; i < 255; i++) {
                                  if(gpiod_line *line = gpiod_chip_get_line(chip, i)) {
                                    const Rec_p row = Obj::to_rec();
                                    const char *consumer = gpiod_line_consumer(line);
                                    row->rec_set("consumer", str(consumer ? consumer : "unused"));
                                    row->rec_set("active", jnt(gpiod_line_active_state(line)));
                                    row->rec_set("direction", GPIOD_LINE_DIRECTION_INPUT == gpiod_line_direction(line)
                                                                  ? vri("input")
                                                                  : vri("output"));
                                    row->rec_set("used", dool(gpiod_line_is_used(line)));
                                    pins->rec_set(vri(to_string(i)), row);
                                    gpiod_line_release(line);
                                  }
                                }
                                gpiod_chip_close(chip);
                              }
#elif defined(ARDUINO)
                              auto lock = lock_guard<Mutex>(gpio_mutex);
                              for(int8_t pin = 0; pin < NUM_DIGITAL_PINS; pin++) {
                                pins->rec_set(vri(to_string(pin)), jnt(digitalRead(pin)));
                              }
#endif
                              return pins;
                            })
                            ->create()},
                       {vri(GPIO_FURI->add_component("write")),
                        InstBuilder::build(GPIO_FURI->add_component("write"))
                            ->domain_range(GPIO_FURI, {1, 1}, GPIO_FURI, {1, 1})
                            ->inst_args(rec({{"value", Obj::to_type(INT_FURI)}}))
                            ->inst_f([](const Obj_p &gpio, const InstArgs &args) {
                              const uint8_t pin = gpio->int_value();
                              const uint8_t value = args->arg("value")->int_value();
                              auto lock = lock_guard<Mutex>(gpio_mutex);
#ifdef NATIVE
                              gpiod_chip *chip = gpiod_chip_open_by_name(GPIO_CHIP_NAME);
                              if(!chip)
                                throw fError::create(gpio->tid->toString(), "unable to open gpio chip %s",
                                                     GPIO_CHIP_NAME);
                              gpiod_line *line = gpiod_chip_get_line(chip, pin);
                              if(!line)
                                throw fError::create(gpio->tid->toString(), "unable to access pin %i on gpio chip %s",
                                                     pin, GPIO_CHIP_NAME);
                              gpiod_line_request_output(line, GPIO_FURI->toString().c_str(), 0);
                              if(gpiod_line_set_value(line, value) < 0) {
                                throw fError::create(gpio->tid->toString(), "unable to write to pin %i on gpio chip %s",
                                                     pin, GPIO_CHIP_NAME);
                              }
                              gpiod_line_release(line);
                              gpiod_chip_close(chip);
#else
                              pinMode(pin, OUTPUT);
                              digitalWrite(pin, value);
#endif
                              return gpio;
                            })
                            ->create()},
                       {vri(GPIO_FURI->add_component("read")),
                        InstBuilder::build(GPIO_FURI->add_component("read"))
                            ->domain_range(GPIO_FURI, {1, 1}, INT_FURI, {1, 1})
                            ->inst_f([](const Obj_p &gpio, const InstArgs &) {
                              int val;
                              const uint8_t pin = gpio->int_value();
                              auto lock = shared_lock<Mutex>(gpio_mutex);
#ifdef NATIVE
                              gpiod_chip *chip = gpiod_chip_open_by_name(GPIO_CHIP_NAME);
                              if(!chip)
                                throw fError::create(gpio->tid->toString(), "unable to open gpio chip %s",
                                                     GPIO_CHIP_NAME);
                              gpiod_line *line = gpiod_chip_get_line(chip, pin);
                              if(!line)
                                throw fError::create(gpio->tid->toString(), "unable to access pin %i on gpio chip %s",
                                                     pin, GPIO_CHIP_NAME);
                              gpiod_line_request_input(line, GPIO_FURI->toString().c_str());
                              if((val = gpiod_line_get_value(line)) < 0) {
                                throw fError::create(gpio->tid->toString(),
                                                     "unable to read from pin %i on gpio chip %s", pin, GPIO_CHIP_NAME);
                              }
                              gpiod_line_release(line);
                              gpiod_chip_close(chip);
#else
                              val = digitalRead(pin);
#endif
                              return jnt(val);
                            })
                            ->create()}});
                ///})(); // call lambda to generate rec
              })
              ->create());
    }
  };
} // namespace fhatos
#endif
