#if !defined(__PROFILER_H__)
#define __PROFILER_H__

//
// Class that profiles overall loop timing
//
class Profiler {
  public:

    Profiler() {}

    void init() {
      noInterrupts();
      TCCR1A = 0;
      TCCR1B = 0;
      TCNT1  = 0;
    
      // (1 tick is 51.2 uSec)
      TIMSK1 |= (1 << TOIE1);                 // enable timer overflow interrupt
      interrupts();                           // enable all interrupts  
    }

    void start() {
      TCNT1  = 0;
      TCCR1B |= (1 << WGM12);                 // CTC mode
      TCCR1B |= (1 << CS10) | (1 << CS12);    // 1024 prescaler
    }

    void stop() {
      TCCR1B = 0;
      totalTicks_ += TCNT1;
    }

    void timerOverflow() {
      totalTicks_ += 65536L;
    }

  private:
    unsigned long totalTicks_ = 0;
};

#endif
