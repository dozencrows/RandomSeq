#include "ShiftRegister.h"

#include <Arduino.h>

void ShiftRegister::step() {
  uint16_t lastBit = value_ & 0x8000;
  value_ <<= 1;
  long randomChoice = random(32768L);
  if (randomChoice < threshold_) {
    lastBit = ~lastBit & 0x8000;
  }
  value_ |= lastBit >> 15;
}

int ShiftRegister::getNote() {
  // 8-bits for full 10V range, like MTM Turing Machine
  uint16_t activeBits = ((value_ & 255) << 3) + 0x100;
  uint16_t fixedValue = activeBits << 5;
  fixedValue /= 17;                           // make this higher e.g. 120 to limit the size of scale
  fixedValue += 0x10;
  return (int)(fixedValue >> 5);
}

void ShiftRegister::write(bool value) {
  
}

void ShiftRegister::setThreshold(long threshold) {
  threshold_ = threshold;
}

void ShiftRegister::setScale(int scale) {
  
}
