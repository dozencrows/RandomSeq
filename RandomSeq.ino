/*
  GMSN! Pure Digit
  14 October 2018
  Nick Tuckett

  Random Sequencer: Turing Machine module emulation

  User interface:
  - In1: clock
  - In2: threshold
  - Out: cv out
  - Encoder:  multiple modes, stepped through by presses:
    - Mode 1: threshold        - position based
    - Mode 2: output octaves   - digit based
    - Mode 3: clocks per step  - 1, 2, 12, 24 or 48
    - Mode 4: write            - turn left writes 0, turn right writes 1
    - Mode 5: quantising scale - position based

  Elements:
  - Clock handler:  500Hz timer interrupt that reads In1, looking for edge. When edge found, it
                    outputs next CV value and sets step flag
  - Shift register: maintains shift register state, with properties:
                      - scale
                      - length
                      - random threshold
                    and methods
                      - step
                      - get value
  - Quantiser:      maps an input value to an output value. Properties:
                      - scale
                    and methods
                      - quantise
  - Main loop:      if step flag set:
                      - clear step flag
                      - if encoder press detected
                        - advance UI mode
                      - process current UI mode
                      - step shift register and get output
                      - get value and quantise
                      - set as next CV out
  - UI mode 1 handler
    - init:
      - set pos to middle (5)
    - select:
      - 
    - process:
      - update position from encoder
      - display position
      - calculate threshold from position and set on shift reg

  - UI mode 2 handler    
    - init:
      - set value to max (9)
    - select:
      - 
    - process:
      - update value from encoder
      - display digit
      - calculate scale from value and set on shift reg

  - UI mode 3 handler    
    - init:
      - set value to max (3)
    - select:
      - 
    - process:
      - update value from encoder
      - display digit
      - calculate clock divisor from value and set on clock

  - UI mode 4 handler    
    - init:
      - 
    - select:
      - set position to middle (5)
    - process:
      - set value to 0
      - update value from encoder
      - if value is -1
        - write 0 into shift register
        - set position to 3
      - if value is 1
        - write 1 into shift register
        - set position to 7
      - if value is 0
        - set position to 5

  - UI mode 5 handler    
    - init:
      - set value to min (0)
    - select:
      - 
    - process:
      - update value from encoder
      - display digit
      - use value to select scale and set on quantiser


  - Profiling indicates that the main loop can be executed in under
    100 microseconds, so the clock sampling interrupt can certainly
    be at a higher frequency - e.g. 4KHz via prescaler at 256 clocks.
*/
#include "PureDigit.h"
#include "ShiftRegister.h"
#include "Quantiser.h"
#include "ClockIO.h"
#include "UIState.h"
#include "Profiler.h"

//
// UI state to manage control of random threshold
//
class UIStateThreshold : public UIState {
  enum { 
    MAX_ENCODER_POS = 23
  };
  
  public:
    UIStateThreshold(PureDigit& digit, ShiftRegister& shiftRegister, Quantiser& quantiser)
      : UIState(digit, shiftRegister, quantiser) {}

    void init() {
      encPos = (MAX_ENCODER_POS + 1) / 2;
    }

    void select() {
      display();
    }

    void update() {
      int newEncPos = digit_.encodeVal(encPos);
      newEncPos = constrain(newEncPos, 0, MAX_ENCODER_POS);

      if (newEncPos != encPos) {
        encPos = newEncPos;
        display();
        long newThreshold = ShiftRegister::MAX_THRESHOLD * (long)encPos / MAX_ENCODER_POS;
        shiftRegister_.setThreshold(newThreshold);
      }
    }

  private:
    int encPos;

    void display() {
      digit_.displayLED(encPos / 2, 0, encPos == 0 || encPos == MAX_ENCODER_POS);
    }
};

//
// UI state to manage control of output scale in octaves
//
class UIStateScale : public UIState {
  enum { 
    MAX_ENCODER_POS = 9
  };
  
  public:
    UIStateScale(PureDigit& digit, ShiftRegister& shiftRegister, Quantiser& quantiser)
      : UIState(digit, shiftRegister, quantiser) {}

    void init() {
      encPos = MAX_ENCODER_POS;
    }

    void select() {
      display();
    }

    void newNote() {
      decimalPoint ^= 1;
    }

    void update() {
      int newEncPos = digit_.encodeVal(encPos);
      newEncPos = constrain(newEncPos, 0, MAX_ENCODER_POS);

      if (newEncPos != encPos) {
        encPos = newEncPos;
        shiftRegister_.setScale(encPos);
      }
      display();
    }

  private:
    int encPos;
    char decimalPoint = 0;

    void display() {
      digit_.displayLED(encPos, 1, decimalPoint);
    }
};

//
// UI state to manage control of clock scale
//
class UIStateClockSteps : public UIState {
  enum { 
    MAX_ENCODER_POS = 4
  };
  
  public:
    UIStateClockSteps(PureDigit& digit, ShiftRegister& shiftRegister, Quantiser& quantiser, ClockIO& clockIO)
      : UIState(digit, shiftRegister, quantiser), clockIO_(clockIO) {}

    void init() {
      encPos = 3;
      clockIO_.setClockDivisor(clocksPerStep_[encPos]);
    }

    void select() {
      display();
    }

    void update() {
      int newEncPos = digit_.encodeVal(encPos);
      newEncPos = constrain(newEncPos, 0, MAX_ENCODER_POS);

      if (newEncPos != encPos) {
        encPos = newEncPos;
        display();
        clockIO_.setClockDivisor(clocksPerStep_[encPos]);
      }
    }

  private:
    int encPos;
    ClockIO& clockIO_;
    static int clocksPerStep_[5];

    void display() {
      digit_.displayLED(encPos, 1, 0);
    }
};

int UIStateClockSteps::clocksPerStep_[5] = { 1, 2, 12, 24, 48 };

//
// UI state to write values into shift register
//
class UIStateClockWrite : public UIState {
  enum { 
    BASE_ENCODER_POS = 6,
    OFFSET_DISPLAY_COUNT = 12000,
  };
  
  public:
    UIStateClockWrite(PureDigit& digit, ShiftRegister& shiftRegister, Quantiser& quantiser)
      : UIState(digit, shiftRegister, quantiser) {}

    void init() {
    }

    void select() {
      displayCount_ = 0;
      digit_.displayLEDChar('-', 0);
    }

    void update() {
      int newEncPos = digit_.encodeVal(BASE_ENCODER_POS);

      if (newEncPos < BASE_ENCODER_POS) {
        shiftRegister_.write(0);
        digit_.displayLED(0, 1, 0);
        displayCount_ = OFFSET_DISPLAY_COUNT;
      } else if (newEncPos > BASE_ENCODER_POS) {
        shiftRegister_.write(1);
        digit_.displayLED(1, 1, 0);
        displayCount_ = OFFSET_DISPLAY_COUNT;
      } else if (displayCount_) {
        displayCount_--;
        if (!displayCount_) {
          digit_.displayLEDChar('-', 0);
        }
      }
    }

  private:
    int displayCount_;
};


PureDigit digit;
ShiftRegister shiftRegister;
Quantiser quantiser;
ClockIO clockIO(digit);

#if defined(PROFILING)
Profiler profiler;
#endif

UIStateThreshold thresholdState(digit, shiftRegister, quantiser);
UIStateScale scaleState(digit, shiftRegister, quantiser);
UIStateClockSteps clockStepsState(digit, shiftRegister, quantiser, clockIO);
UIStateClockWrite writeState(digit, shiftRegister, quantiser);

UIState* uiStates[] = { &thresholdState, &scaleState, &clockStepsState, &writeState };

int currentUIMode = 0;

int debounceCount = 0;
bool switchDownCheck = false;

void setup() {
  digit.dontCalibrate();
  digit.begin();
  digit.dacWrite(quantiser.getCV());
  quantiser.setScale(Quantiser::Scale::MAJOR);
  quantiser.setNote(shiftRegister.getNote());
  clockIO.init();
  clockIO.setNextCvOut(quantiser.getCV());
#if defined(PROFILING)
  profiler.init(120000L);
#endif  
  thresholdState.init();
  scaleState.init();
  
  clockIO.start();
  uiStates[0]->select();

#if defined(PROFILING)
  Serial.begin(9600);  
  Serial.println("###");
  profiler.start();  
#endif
}

#if defined(PROFILING)
ISR(TIMER1_OVF_vect) {
  profiler.timerOverflow();
}
#endif

ISR(TIMER2_COMPA_vect) {
  clockIO.update();
}

void selectNextNote() {
  shiftRegister.step();
  quantiser.setNote(shiftRegister.getNote());
  clockIO.setNextCvOut(quantiser.getCV());
  scaleState.newNote();
}

void loop() {
  UIState* currentUIState = uiStates[currentUIMode];

  if (digit.getSwitchState() != 0 && !switchDownCheck) {
    debounceCount = 20;
    switchDownCheck = true;
  } else if (debounceCount > 0) {
    debounceCount--;
    if (debounceCount == 0) {
      if (digit.getSwitchState() != 0) {
        currentUIMode++;
        if (currentUIMode >= sizeof(uiStates) / sizeof(UIState*)) {
          currentUIMode = 0;
        }
        currentUIState = uiStates[currentUIMode];
        currentUIState->select();
      } else {
        switchDownCheck = false;
      }
    }
  } else if (switchDownCheck && digit.getSwitchState() == 0) {
    switchDownCheck = false;
  }

  currentUIState->update();
  
  if (clockIO.hasStepTicked()) {
    clockIO.ackStepTick();
    selectNextNote();
  }

#if defined(PROFILING)
  profiler.update();
#endif
}
