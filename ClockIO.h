#if !defined(__CLOCKIO_H__)
#define __CLOCKIO_H__

// Set this to force the clock to step at a given number of
// interrupts, for debugging/profiling purposes on an
// Arduino board
//#define DEBUG_CLOCK_RATE 100

//
// Class that manages clock input, CV input, overall timing and CV output
//
class ClockIO {
  public:

    ClockIO(PureDigit& digit) : digit_(digit) {}

    void init() {
      noInterrupts();
      TCCR2A = 0;
      TCCR2B = 0;
      TCNT2  = 0;
    
      // (1 tick is 12.8 uSec)
      OCR2A = 19;                             // Approx 4KHz timer
      TIMSK2 |= (1 << OCIE1A);                // enable timer compare interrupt
      interrupts();                           // enable all interrupts  
    }

    void start() {
      TCCR2B |= (1 << WGM12);                 // CTC mode
      TCCR2B |= (1 << CS12);                  // 256 prescaler
    }

    void stop() {
      TCCR2B = 0;
    }

    void setClockDivisor(int clockDivisor) {
      clockDivisor_ = clockDivisor;
      clockCount_ = 0;
    }

    void setNextCvOut(int cvOut) {
      nextCvOut = cvOut;
    }

    bool hasStepTicked() {
      return stepTicked_;
    }

    void ackStepTick() {
      stepTicked_ = false;
    }

    int getCv2In() {
      return cv2In_;
    }

    void update() {
      int clock = digit_.adcRead(1);
      // Clock triggers next note on rising edge, threshold around 0.5V
#if defined(DEBUG_CLOCK_RATE)
      static int debugClockCount = 0;
      debugClockCount++;
      if (debugClockCount >= DEBUG_CLOCK_RATE) {
#else
      if (clock < 2000 && lastClock > 2000) {
#endif
        clockCount_++;
        if (clockCount_ >= clockDivisor_) {
          clockCount_ = 0;
          digit_.dacWrite(nextCvOut);
          stepTicked_ = true;
        }
      }
      lastClock = clock;

      cv2In_ = digit_.adcRead(2);
    }

  private:
    PureDigit& digit_;
    bool stepTicked_ = false;
    int nextCvOut = 0;
    int lastClock = 0;  
    int clockDivisor_ = 1;
    int clockCount_ = 0;
    int cv2In_ = 0;
};

#endif
