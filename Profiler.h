#if !defined(__PROFILER_H__)
#define __PROFILER_H__

//
// Class that profiles overall loop timing
//
class Profiler {
  public:

    Profiler() {}

    void init(unsigned long measurementCount) {
      noInterrupts();
      TCCR1A = 0;
      TCCR1B = 0;
      TCNT1  = 0;
    
      TIMSK1 |= (1 << TOIE1);                 // enable timer overflow interrupt
      interrupts();                           // enable all interrupts  
      measurementCount_ = measurementCount;
    }

    void start() {
      TCNT1  = 0;
      TCCR1B |= (1 << CS10);                  // 1 tick per cycle
    }

    void update() {
      if (measurementCount_) {
        measurementCount_--;
        if (!measurementCount_) {
          TCCR1B = 0;
          totalTicks_ += TCNT1;
          Serial.println(totalTicks_);
        }
      }
    }

    void timerOverflow() {
      totalTicks_ += 65536L;
    }

  private:
    unsigned long totalTicks_ = 0;
    unsigned long measurementCount_;
};

#endif
