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
#include <avr/pgmspace.h>
#include <PureDigit.h>
#include "ShiftRegister.h"

PureDigit digit;
ShiftRegister shiftRegister;

//Setup variables
int encPos = 0;
int lastEncPos = 0;
const int noteVoltsLookUp[] PROGMEM = {
  2047, 2030, 2013, 1996, 1979, 1962, 1945, 1928, 1911, 1893, 1876, 1859, 1842, 1825, 1808, 1791,
  1774, 1757, 1740, 1723, 1706, 1689, 1672, 1655, 1638, 1621, 1603, 1586, 1569, 1552, 1535, 1518,
  1501, 1484, 1467, 1450, 1433, 1416, 1399, 1382, 1365, 1348, 1331, 1313, 1296, 1279, 1262, 1245,
  1228, 1211, 1194, 1177, 1160, 1143, 1126, 1109, 1092, 1075, 1058, 1041, 1024, 1006, 989, 972,
  955, 938, 921, 904, 887, 870, 853, 836, 819, 802, 785, 768, 751, 734, 716, 699,
  682, 665, 648, 631, 614, 597, 580, 563, 546, 529, 512, 495, 478, 461, 444, 426,
  409, 392, 375, 358, 341, 324, 307, 290, 273, 256, 239, 222, 205, 188, 171, 154,
  136, 119, 102, 85, 68, 51, 34, 17, 0
}; 
const char majorScale[] PROGMEM = {
  0, 2, 4, 5, 7, 9, 11, 12
};

const char minorScale[] PROGMEM = {
  0, 2, 3, 5, 7, 8, 10, 12  
};

int nextNoteIndex = 0;
int nextCvOut = 0;
int noteIndex = 0;
bool noteUpdated = 0;
bool lastSwitchState = 0;
const char* currentScale = majorScale;

void setup() {
  digit.dontCalibrate();
  digit.begin();
  digit.dacWrite(pgm_read_word_near(noteVoltsLookUp));
  selectNextNote(currentScale);
  
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  // (1 tick is 51.2 uSec)
//  OCR1A = 19532;                          // compare match register 20MHz/1024/1Hz = 19532
  OCR1A = 4883;                           // compare match register 20MHz/1024/4Hz = 4883
  TIMSK1 |= (1 << OCIE1A);                // enable timer compare interrupt
  interrupts();                           // enable all interrupts  
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

void selectNextNote(const char* scale) {
  shiftRegister.step();
  int note = shiftRegister.getNote();
  int octave = note / 12;
  int semitone = note % 12;

  for (int i = 0; i < 8; i++) {
    int scaleSemitone = pgm_read_byte_near(scale + i);
    if (semitone <= scaleSemitone) {
      semitone = scaleSemitone;
      break;
    }
  }

  if (semitone == 12) {
    octave++;
    semitone = 0;
  }
  
  int noteLookup = octave * 12 + semitone;
  nextCvOut = pgm_read_word_near(noteVoltsLookUp + noteLookup);
  nextNoteIndex = semitone;
}

void showNote(int semitone) {
  if (semitone < 10) {
    digit.displayLED(semitone, 1, 0);
  } else {
    digit.displayLED(semitone - 10, 1, 1);
  } 
}

void loop() {
  encPos = digit.encodeVal(encPos);
  encPos = constrain(encPos, 0, 1);

  if (digit.getSwitchState() != 0) {
    if (!lastSwitchState) {
      lastSwitchState = 1;
      if (currentScale == majorScale) {
        currentScale = minorScale;
      } else {
        currentScale = majorScale;
      }
    }
  } else {
    lastSwitchState = 0;
  }

  if (encPos == 1) {
    if (!lastEncPos) {
      startTimer();
    }
    if (noteUpdated) {
      noteUpdated = 0;
      noteIndex = nextNoteIndex;      
      selectNextNote(currentScale);
    }
  } else {
    if (lastEncPos) {
      stopTimer();
    }
    noteUpdated = 0;
  }
  
  showNote(noteIndex);
  lastEncPos = encPos;

  delay(10);
}
