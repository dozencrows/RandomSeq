#include "ShiftRegister.h"
#include <Arduino.h>
#include <avr/pgmspace.h>

// Divisors to scale the full 2047 across 1 to 10 octaves
static const uint16_t scaleDivisors[] PROGMEM = {
  170, 85, 57, 43, 34, 28, 24, 21, 19, 17
};

void ShiftRegister::step() {
  uint16_t lastBit = value_ >> 15;
  value_ <<= 1;
  long randomChoice = random(32768L);
  if (randomChoice < threshold_) {
    lastBit ^= 1;
  }
  value_ |= lastBit;
}

int ShiftRegister::getNote() {
  // 8-bits for full 10 octave range, like MTM Turing Machine
  uint16_t activeBits = ((value_ & 255) << 3) + 0x100;
  uint16_t fixedValue = activeBits << 5;
  fixedValue /= scaleDivisor_;
  fixedValue += 0x10;
  return (int)(fixedValue >> 5);
}

void ShiftRegister::write(bool value) {
  if (value) {
    value_ |= 1;
  } else {
    value_ &= 0xfffe;
  }
}

void ShiftRegister::setThreshold(long threshold) {
  threshold_ = threshold;
}

void ShiftRegister::setScale(int scale) {
  scaleDivisor_ = pgm_read_word_near(scaleDivisors + scale);
}
