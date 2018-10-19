/*
 * Shift register emulation
 */
#if !defined(__SHIFTREGISTER_H__)
#define __SHIFTREGISTER_H__

#include <stdint.h>

class ShiftRegister {
  public:
    void step();
    int getNote();
    void write(bool value);

    void setThreshold(int threshold);
    void setScale(int scale);

  private:
    uint16_t value_ = 0;
    int threshold_ = 16384;
    // scale
};

#endif
