/*
 * Shift register emulation
 */
#if !defined(__SHIFTREGISTER_H__)
#define __SHIFTREGISTER_H__

#include <stdint.h>

class ShiftRegister {
  public:
    enum { 
      MAX_THRESHOLD = 32768
    };
    
    void step();
    int getNote();
    void write(bool value);

    void setThreshold(long threshold);
    void setScale(int scale);

  private:
    uint16_t value_ = 0;
    long threshold_ = 16384L;
    // scale
};

#endif
