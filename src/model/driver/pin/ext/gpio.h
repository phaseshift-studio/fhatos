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
#ifdef NATIVE
#include <gpiod.h>

#define INPUT             0x01
// Changed OUTPUT from 0x02 to behave the same as Arduino pinMode(pin,OUTPUT) 
// where you can read the state of pin even when it is set as OUTPUT
#define OUTPUT            0x03 
#define PULLUP            0x04
#define INPUT_PULLUP      0x05
#define PULLDOWN          0x08
#define INPUT_PULLDOWN    0x09
#define OPEN_DRAIN        0x10
#define OUTPUT_OPEN_DRAIN 0x13
#define ANALOG            0xC0

void digitalWrite(const int pin, const int value) {
  /*struct gpiod_chip *chip = gpiod_chip_open_by_name("gpiochip0");
  struct gpiod_line *line = gpiod_chip_get_line(chip, pin);
  gpiod_line_request_output(line, "example1", value);*/
}

int digitalRead(const int pin) {
 /* struct gpiod_chip *chip = gpiod_chip_open_by_name("gpiochip0");
  struct gpiod_line *line = gpiod_chip_get_line(chip, pin);
  return  gpiod_line_request_input(line, "example1");*/
}

void pinMode(const int pin, const int mode) {

}

#endif
#endif