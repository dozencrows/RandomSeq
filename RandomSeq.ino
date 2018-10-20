/*
  GMSN! Pure Digit
  14 October 2018
  cc-by 4.0
  Nick Tuckett

  Random Sequencer: Turing Machine module emulation

  User interface:
  - In1: threshold
  - In2: clock
  - Out: cv out
  - Encoder:  multiple modes, stepped through by presses:
    - Mode 1: threshold        - position based
    - Mode 2: output scale     - position based
    - Mode 3: write            - turn left writes 0, turn right writes 1
    - Mode 4: quantising scale - position based

  Elements:
  - Clock handler:  1kHz timer interrupt that reads In2, looking for edge. When edge found, it
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

  - UI mode 4 handler    
    - init:
      - set value to min (0)
    - select:
      - 
    - process:
      - update value from encoder
      - display digit
      - use value to select scale and set on quantiser


*/
#include <PureDigit.h>
#include "ShiftRegister.h"
#include "Quantiser.h"
#include "UIState.h"

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

PureDigit digit;
ShiftRegister shiftRegister;
Quantiser quantiser;
UIStateThreshold thresholdState(digit, shiftRegister, quantiser);

int encPos = 0;
int lastEncPos = 0;
Quantiser::Scale scale = Quantiser::Scale::CHROMATIC;

int nextNoteIndex = 0;
int nextCvOut = 0;
int noteIndex = 0;
bool noteUpdated = 0;
bool lastSwitchState = 0;

void setup() {
  digit.dontCalibrate();
  digit.begin();
  digit.dacWrite(quantiser.getCV());
  selectNextNote();
  thresholdState.init();
  
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  // (1 tick is 51.2 uSec)
//  OCR1A = 19532;                          // compare match register 20MHz/1024/1Hz = 19532
  OCR1A = 3000;                           // compare match register 20MHz/1024/4Hz = 4883
  TIMSK1 |= (1 << OCIE1A);                // enable timer compare interrupt
  interrupts();                           // enable all interrupts  

  startTimer();
  thresholdState.select();
}

ISR(TIMER1_COMPA_vect) {
  digit.dacWrite(nextCvOut);
  noteUpdated = 1;
}

void startTimer() {
  TCCR1B |= (1 << WGM12);                 // CTC mode
  TCCR1B |= (1 << CS10) | (1 << CS12);    // 1024 prescaler
}

void stopTimer() {
  TCCR1B = 0;
}

void selectNextNote() {
  shiftRegister.step();
  quantiser.setNote(shiftRegister.getNote());
  nextCvOut = quantiser.getCV();
  nextNoteIndex = quantiser.getSemitone();
}

void showNote(int semitone) {
  if (semitone < 10) {
    digit.displayLED(semitone, 1, 0);
  } else {
    digit.displayLED(semitone - 10, 1, 1);
  } 
}

void loop() {
  if (digit.getSwitchState() != 0) {
    if (!lastSwitchState) {
      lastSwitchState = 1;
      switch (scale) {
        case Quantiser::Scale::CHROMATIC:
          scale = Quantiser::Scale::MAJOR;
          break;
        case Quantiser::Scale::MAJOR:
          scale = Quantiser::Scale::MINOR;
          break;
        case Quantiser::Scale::MINOR:
          scale = Quantiser::Scale::CHROMATIC;
          break;
      }
      quantiser.setScale(scale);
    }
  } else {
    lastSwitchState = 0;
  }

  thresholdState.update();
  
  if (noteUpdated) {
    noteUpdated = 0;
    noteIndex = nextNoteIndex;      
    selectNextNote();
  }

  delay(10);
}
