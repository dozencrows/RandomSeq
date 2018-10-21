#if !defined(__CLOCKIO_H__)
#define __CLOCKIO_H__

//
// Class that manages clock input, overall timing and CV output
//
class ClockIO {
  public:

    ClockIO(PureDigit& digit) : digit_(digit) {}

    void init() {
      noInterrupts();
      TCCR1A = 0;
      TCCR1B = 0;
      TCNT1  = 0;
    
      // (1 tick is 51.2 uSec)
      OCR1A = 38;                             // Approx 500Hz timer
      TIMSK1 |= (1 << OCIE1A);                // enable timer compare interrupt
      interrupts();                           // enable all interrupts  
    }

    void start() {
      TCCR1B |= (1 << WGM12);                 // CTC mode
      TCCR1B |= (1 << CS10) | (1 << CS12);    // 1024 prescaler
    }

    void stop() {
      TCCR1B = 0;
    }

    void setNextCvOut(int cvOut) {
      nextCvOut = cvOut;
    }

    bool hasClockTicked() {
      return clockTicked;
    }

    void ackClockTick() {
      clockTicked = false;
    }

    void update() {
      // Clock triggers next note on falling edge, threshold around 2.5V
      int clock = -(digit_.adcRead(0) - 2048);
      if (clock < 400 && lastClock > 512) {
        digit_.dacWrite(nextCvOut);
        clockTicked = true;
      }
      lastClock = clock;
    }

  private:
    PureDigit& digit_;
    bool clockTicked = 0;
    int nextCvOut = 0;
    int lastClock = 0;  
};

#endif
