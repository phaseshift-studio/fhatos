#pragma once
#ifndef fhatos_oled_hpp
#define fhatos_oled_hpp

#ifdef BLAH

#include "../../../fhatos.hpp"
#include "ext/ss_oled.hpp"

namespace fhatos {

// let ss_oled find the address of our display
#define OLED_ADDR -1
// Use the default Wire library
#define USE_HW_I2C 0
#define BLANK_ROW (char *)"                      "

  class OLED {
OLED(const uint8_t sclPin, const uint8_t sdaPin,
               const uint8_t i2cAddr, const int rotate,
               const int screenSize) {
  int rc =
      oledInit(&xscreen, screenSize, i2cAddr, rotate != FAT_ROTATE::ROTATE_0, 0,
               USE_HW_I2C, sdaPin, sclPin, -1, 400000L);
  this->clear();
  if (onStart)
    onStart(this);
  if (rc != (int)OLED_NOT_FOUND) {
    LOG(INFO, "[Screen Configuration]\n");
    LOG(INFO,
        "\tPins      : [clock:" FP_GPIOD_STR "]/[data:" FP_GPIOD_STR "]\n"
        "\tAddress   : " FP_I2C_ADDR_STR "\n"
        "\tDimensions: %ix%i\n"
        "\tRotation  : %i degrees\n",
        FP_GPIOD(sclPin), FP_GPIOD(sdaPin), FP_I2C_ADDR(xscreen.oled_addr),
        xscreen.oled_x, xscreen.oled_y, (int)rotate);
  } else {
    LOG(ERROR,
        "OLED not found at [i2c:0x%x/%i] on I2C pins [clock:" FP_GPIOD_STR
        "] and [data:" FP_GPIOD_STR "]\n",
        i2cAddr, i2cAddr, FP_GPIOD(sclPin), FP_GPIOD(sdaPin));
  }
#if FP_TESTING == FP_HARDWARE
  const FAT_FONT font = FAT_FONT::FONT_6x8;
  this->loopFunction = [this, font]() {
    for (int i = 0; i < 10; i++) {
      this->writef(i, font, "loop [row:%i]", i);
    }
  };
  this->updateFunction = [this, font]() {
    this->writef(0, font, "[oled:%i][font:%i]", this->xscreen.oled_type,
                 (uint8_t)font);
    for (int i = 1; i < 10; i++) {
      this->writef(i, font, "update [row:%i]", i);
    }
  };
#endif
}

void clear() { oledFill(&xscreen, 0, 1); }

void clear(const int8_t row, const FAT_FONT font) {
  write(row, 0, font, BLANK_ROW, false);
}

void clear(const int8_t row, const int8_t col, const FAT_FONT font) {
  write(row, col, font, BLANK_ROW);
}

void write(const int8_t row, const FAT_FONT font, const char *text,
                   const bool invert, const bool preClear) {
  if (preClear)
    clear(row, font);
  write(row, 0, font, text, invert);
}

void write(const int8_t row, const int8_t col, const FAT_FONT font,
                   const char *text, const bool invert) {
  oledWriteString(&xscreen, 0, col, row, (char *)text, font, invert ? 1 : 0, 1);
}
};
}

#endif
#endif